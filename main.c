#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 100


int8_t SREG = 0;

#define PC_SIZE 16
short int unsigned PC = 0;

unsigned int Clock = 0;

short int InstructionMemory [1024];
int8_t DataMemory[2048];
int8_t GPRS[64];

typedef struct {
    int8_t opCode;
    int8_t operand1;
    int8_t operand2;
    short int unsigned pc;
} DecodedInstruction;

typedef struct {
    short int instruction;
    short int unsigned pc;
} Instruction;


DecodedInstruction instructionToBeExecuted = {0, 0, 0, 0};
DecodedInstruction delayExec;
Instruction instructionToBeDecoded = {0, 0};
Instruction delayDecode;



int check_overflow(int value1, int value2, int result) {

    bool sign_value1 = (value1 & 0x80) != 0;
    bool sign_value2 = (value2 & 0x80) != 0;
    bool sign_result = (result & 0x80) != 0;

    // Overflow occurs if the signs of value1 and value2 are the same,
    // but the sign of the result is different.
    if ((sign_value1 == sign_value2) && (sign_result != sign_value1)) {
        return 1; // Overflow occurred
    }

    return 0; // No overflow
}

int check_carry(uint8_t value1, uint8_t value2, uint8_t result) {
    // Carry occurs if the result is less than either of the operands
    if (result < value1 || result < value2) {
        return 1; // Carry occurred
    }

    return 0; // No carry
}

void printBinary(int8_t number) {
    for (int i = 7; i >= 0; i--) {
        int bit = (number >> i) & 1;
        printf("%d", bit);
    }
    printf("\n");
}

short parse_instruction(char *assemblyCode) {

    char *token;
    char *delim = " ";
    token = strtok(assemblyCode, delim);
    short binaryCode = 0;

    if (strcmp(token, "ADD") == 0) {
        binaryCode = 1;
    } else if (strcmp(token, "SUB") == 0) {
        binaryCode = 2;
    } else if (strcmp(token, "MUL") == 0) {
        binaryCode = 3;
    } else if (strcmp(token, "MOVI") == 0) {
        binaryCode = 4;
    } else if (strcmp(token, "BEQZ") == 0) {
        binaryCode = 5;
    } else if (strcmp(token, "ANDI") == 0) {
        binaryCode = 6;
    } else if (strcmp(token, "EOR") == 0) {
        binaryCode = 7;
    } else if (strcmp(token, "BR") == 0) {
        binaryCode = 8;
    } else if (strcmp(token, "SAL") == 0) {
        binaryCode = 9;
    } else if (strcmp(token, "SAR") == 0) {
        binaryCode = 10;
    } else if (strcmp(token, "LDR") == 0) {
        binaryCode = 11;
    } else if (strcmp(token, "STR") == 0) {
        binaryCode = 12;
    }

    int maskOperand = 0b00000000000000000000000000111111;
    token = strtok(NULL, delim);
    int firstOperand = (atoi(token + 1)) & maskOperand;
    token = strtok(NULL, delim);
    int secondOperand;

    if (token[0] == 'R') {
        secondOperand = atoi(token + 1);
    } else {
        secondOperand = atoi(token);
    }

    secondOperand = secondOperand & maskOperand;
    binaryCode = (binaryCode << 6) | firstOperand;
    binaryCode = (binaryCode << 6) | secondOperand;

    return binaryCode;
}


void load_instructions(const char *filename) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int instructionIndex = 0;

    while (fgets(line, sizeof(line), file) != NULL && instructionIndex < 1024) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = '\0';
        short binaryCode = parse_instruction(line);
        InstructionMemory[instructionIndex++] = binaryCode;
    }

    fclose(file);
}

Instruction fetch(){

    short int currInstruction = InstructionMemory[PC];
    printf("PC: %d\n", PC);
    Instruction i;
    i.instruction = currInstruction;
    i.pc = PC;
    PC++;
    return i;

}

DecodedInstruction Decode(Instruction instruction){
    DecodedInstruction decoded;
    decoded.opCode   = (instruction.instruction & 0b1111000000000000) >> 12;
    decoded.operand1 = (instruction.instruction & 0b0000111111000000) >> 6;
    decoded.operand2 = (instruction.instruction  & 0b0000000000111111);
    decoded.pc = instruction.pc;
    return decoded;
}

void Execute(DecodedInstruction decodedInstruction){


    int8_t opCode = decodedInstruction.opCode;
    int8_t operand1 = decodedInstruction.operand1;
    int8_t operand2 = decodedInstruction.operand2;
    short int instrPC = decodedInstruction.pc;
    int8_t num1;
    int8_t num2;
    int8_t result;
    int msb1;
    int msb2;
    int msbResult;
    SREG = 0;
    
    switch(opCode){

        case 1:
            num1 = GPRS[operand1];
            msb1 = ((num1) & 0b100000) >> 5;

            num2 = GPRS[operand2];
            msb2 = ((num2) & 0b100000) >> 5;

            result = num1 + num2;
            msbResult = ((result) & 0b1000000) >> 6;
            
            
            //Carry exists
            if(check_carry(num1,num2,result)){
                SREG = SREG | 0b00010000;
            }
            
            //Overflow
            if(check_overflow(num1, num2, result) == 1){
                SREG = SREG | 0b00001000;
            }

            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }

            //Sign flag
            if((check_overflow(num1, num2, result) == 1) ^ result<0){
                SREG = SREG | 0b00000010;
            }

            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            
            
            
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , R%d = %d\n", operand1, num1, operand2, num2);
            printf("Operation ADD : R%d = R%d + R%d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 2:
            num1 = GPRS[operand1];
            num2 = GPRS[operand2];
            result = num1 - num2;

            
            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }

             //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }

            //Overflow
            if(check_overflow(num1, num2, result) == 1){
                SREG = SREG | 0b00001000;
            }
            //Sign flag
            if((check_overflow(num1, num2, result) == 1) ^ result<0){
                SREG = SREG | 0b00000010;
            }

            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , R%d = %d\n", operand1, num1, operand2, num2);
            printf("Operation SUB : R%d = R%d - R%d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 3:
            num1 = GPRS[operand1];
            num2 = GPRS[operand2];
            result = num1 * num2;

            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }

            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , R%d = %d\n", operand1, num1, operand2, num2);
            printf("Operation MUL : R%d = R%d * R%d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 4:
            GPRS[operand1] = operand2;
            printf("Operation MOVI : R%d = %d\n", operand1, operand2);
            break;
        case 5:
            int num1 = GPRS[operand1];
            printf("Operation BEQZ : IF(R%d==0){\n\t PC + 1 + %d}\n This Instruction PC was : %d\n", operand1, operand2, instrPC);
            if(num1 == 0){
                instructionToBeDecoded.instruction = 0;
                instructionToBeExecuted.opCode = 0;
                delayExec.opCode = 0;
                delayDecode.instruction = 0;
                result = instrPC + 1 + operand2; //instrPC is the address of the branch instruction itself without incrementing the PC
                PC = result;
                printf("New Value of PC = %d\n", result);
                printf("Pipeline flushed!\n");
            }
            break;
        case 6:
             num1 = GPRS[operand1];
             result = num1 & operand2;

            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }
            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , IMM = %d\n", operand1, num1, operand2);
            printf("Operation ANDI : R%d = R%d & %d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 7:
             num1 = GPRS[operand1];
             num2 = GPRS[operand2];
             result = num1 ^ num2;

            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }
            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , R%d = %d\n", operand1, num1, operand2, num2);
            printf("Operation EOR : R%d = R%d XOR R%d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 8:
            num1 = GPRS[operand1];
            num2 = GPRS[operand2];
            result = (num1 << 6) | (num2);
            PC = result;
            instructionToBeDecoded.instruction = 0;
            instructionToBeExecuted.opCode = 0;
            delayExec.opCode = 0;
            delayDecode.instruction = 0;
            
            printf("Old Values : R%d = %d , R%d = %d\n", operand1, num1, operand2, num2);
            printf("Operation BR : PC = R%d CONCAT R%d , PC = %d\n", operand1, operand2, PC);
            printf("New Value of PC = %d\n", result);
            printf("Pipeline flushed!\n");
            break;
        case 9:
             num1 = GPRS[operand1];
             result = num1 << operand2;
            
            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }
            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , IMM = %d\n", operand1, num1, operand2);
            printf("Operation SAL : R%d = R%d << %d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 10:
            num1 = GPRS[operand1];
            result = (num1 >> operand2);

            //Negative flag
            if(result<0){
                SREG = SREG | 0b00000100;
            }
            //Zero flag
            if(result == 0){
                SREG = SREG | 0b00000001;
            }
            GPRS[operand1] = result;
            printf("Old Values : R%d = %d , IMM = %d\n", operand1, num1, operand2);
            printf("Operation SAR : R%d = R%d >> %d\n", operand1, operand1, operand2);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 11:
            result = DataMemory[operand2];
            GPRS[operand1] = result;
            printf("Operation LDR : R%d = MEM[%d]\n", operand1, operand1);
            printf("New Value of R%d = %d\n", operand1, result);
            break;
        case 12:
            DataMemory[operand2] = GPRS[operand1];
            printf("Operation STR : MEM[%d] = R%d \n", operand2, operand1);
            printf("New Value of MEM[%d] = %d\n", operand2, GPRS[operand1]);
            break;

    }
    printf("SREG: XXXCVNSZ\n      ");
    printBinary(SREG);
    return;

}

int main(){

    


    load_instructions("instructions.txt");
    GPRS[1] = 10;
    GPRS[2] = 5;
    GPRS[3] = 20;
    GPRS[4] = 4;
    GPRS[5] = 6;
    GPRS[6] = 7;
    GPRS[7] = 15;
    GPRS[8] = 9;
    GPRS[9] = 5;
    GPRS[10] = 42;
    GPRS[11] = 0;
    GPRS[12] = 3;
    GPRS[13] = -16;
    DataMemory[20] = 55;
    GPRS[14] = 75;
    GPRS[15] = 0;
    GPRS[16] = 15;
    GPRS[17] = 30;
    GPRS[18] = 12;

    
    int NumberofInstructions = 0;
    for(int i=0; i<1024;i++){
        if(InstructionMemory[i]!=0)
            NumberofInstructions++;
        else break;
    }
    
    
    printf("--------------------------------------------------------------------\n");
    

    int cycle = 1;
    bool flag = true;
    int counter = 0;
    while(flag)
    {   
        
        printf("Cycle %d\n\n",cycle);
        if(PC<1024){
            if(InstructionMemory[PC] != 0)
                printf("Fetched Instruction %d\n" ,PC);
            delayDecode = fetch();     
            }
            else
                delayDecode.instruction = 0;
            
        if(instructionToBeDecoded.instruction != 0){
            delayExec = Decode(instructionToBeDecoded);
            printf("Decoded Instruction %d\n" ,instructionToBeDecoded.pc);
        }
        if(instructionToBeExecuted.opCode !=0){
            Execute(instructionToBeExecuted);  
            printf("Executed Instruction %d\n" ,instructionToBeExecuted.pc);
        }
     
        instructionToBeDecoded.instruction = delayDecode.instruction;
        instructionToBeDecoded.pc = delayDecode.pc;
        if(delayExec.opCode != 0){
            instructionToBeExecuted.opCode = delayExec.opCode;
            instructionToBeExecuted.operand1 = delayExec.operand1;
            instructionToBeExecuted.operand2 = delayExec.operand2;
            instructionToBeExecuted.pc = delayExec.pc;
        }
        
        printf("----------------------------Cycle %d ended------------------------------\n", cycle);
        cycle++;
        if(InstructionMemory[PC] == 0){
            if(counter == 2){
                flag = false;
            }
            counter++;
        }
        
    }
    printf("##############################################################################\n");
    printf("----------------------------All Registers------------------------------\n");
    
    for (int j = 0; j < 64; j++) {
        printf("Register %d: %d | ", j, GPRS[j]);
    }
    
    printf("\n##############################################################################\n");
    printf("----------------------------Instruction Memory------------------------------\n");

    for(int i = 0; i < 1024;i++){
        if(InstructionMemory[i] != 0)
            printf("Instruction %d: %d | ", i, InstructionMemory[i]);
    }


    printf("\n##############################################################################\n");
    printf("----------------------------Data Memory------------------------------\n");
    
    for(int i = 0; i < 2048;i++){
        if(DataMemory[i] != 0)
            printf("MEM[%d]: %d | ", i, DataMemory[i]);
    }
    printf("\n##############################################################################\n");

    // for (int i = 0; i < 1024; i++) {
    //     if (InstructionMemory[i] != 0) { // Assuming 0 means an empty slot
    //         printf("Instruction %d: %d\n", i, InstructionMemory[i]);
    //         //fetch(InstructionMemory[i]);
    //         //printf("Clock Cycle : %d\n", Clock);
    //         for (int j = 0; j < 64; j++) {
    //             if (GPRS[j] != 0) { // Assuming 0 means an empty slot
    //                 printf("Register %d: %d\n", j, GPRS[j]);
    //             }
    //         }
    //         printf("-------------------------------------------------------------\n");
    //     }
        
    // }
    // printf("-------------------------------------------------------------\n\t\t\tProgram Terminated\n");
    return 0;

}