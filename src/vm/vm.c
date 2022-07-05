#include "./vm.h"
#include <stdio.h>

void handle_input_output(uint16_t instruction){
    switch((instruction & 0x0FC0) >> 6){
        case PRINT:
            switch(instruction & 0x003F){
                case INTEGER:
                    printf("%d", stack[SP-1]);
                    break;
            }
            --SP;
            break;
        case PRINT_ESEQ:
            switch(instruction & 0x003F){
                case NEWLINE:
                    putchar('\n');
                    break;
                case RETURN_CARRIAGE:
                    putchar('\r');
                    break;
            }
            break;
        default:
            reg_data[Rerr] = ILLEGAL_PARAMETER;
    }
}

void handle_arithmetic(uint16_t instruction){
    switch(instruction & 0x0FFF){
        case ADD:
            stack[SP-2] = (stack[SP-1] + stack[SP-2]);
            break;
        case SUB:
            stack[SP-2] = (stack[SP-1] - stack[SP-2]);
            break;
        case MULT:
            stack[SP-2] = (stack[SP-1]*stack[SP-2]);
            break;
        case DIV:
            stack[SP-2] = (uint16_t) (stack[SP-1]/stack[SP-2]);
            break;
        default:
            reg_data[Rerr] = ILLEGAL_PARAMETER;
            break;
    }
}

void handle_logic(uint16_t instruction){
    switch(instruction & 0x0FFF){
        case AND:
            stack[SP-2] = stack[SP-1] & stack[SP-2];
            --SP;
            break;
        case OR:
            stack[SP-2] = stack[SP-1] | stack[SP-2];
            --SP;
            break;
        case NOT:
            stack[SP-1] = ~stack[SP-1];
            break;
        default:
            reg_data[Rerr] = ILLEGAL_PARAMETER;
    }
}

void handle_bit_shift(uint16_t Instruction){
    uint16_t direction = ((Instruction & 0x0F00) >> 8);
    uint16_t count = (Instruction & 0x00FF);
    switch(direction){
        case LEFT:
            stack[SP-1] = (stack[SP-1] << count);
            break;
        case RIGHT:
            stack[SP-1] = (stack[SP-1] >> count);
            break;
        default:
            reg_data[Rerr] = ILLEGAL_PARAMETER;
    }
    SP++;
}

void handle_reg_storage(uint16_t reg_index){
    switch(reg_index){
        case Ra:
        case Rb:
        case Rc:
        case Rip:
            reg_data[reg_index] = stack[SP--];
            break;
        case Rcbindx:
            reg_data[Rbindx] &= 0xFF00;
            reg_data[Rbindx] |= stack[SP--];
            break;
        case Rvbindx:
            reg_data[Rbindx] &= 0x00FF;
            reg_data[Rbindx] |= stack[SP--];
            break;
        case Rhlt:
        case Rerr:
        case Rcom:
        default:
            reg_data[Rerr] = REGISTER_ACCESS_DENIED;
            break;
    }
}

void handle_reg_load(uint16_t reg_index){
    switch(reg_index){
        case Rcbindx:
            stack[SP++] = (reg_data[Rbindx] & 0x00FF);
            break;
        case Rvbindx:
            stack[SP++] = (reg_data[Rbindx] >> 8);
            break;
        default:
            stack[SP++] = reg_data[reg_index];
            break;
    }
}

void execute_instruction(uint16_t instruction){

    switch(OPCODE(instruction)){
        case PUSH:
            stack[SP++] = (instruction & 0x0FFF);
            break;
        case POP:
            reg_data[Ra] = stack[--SP];
            break;
        case DUP:
            stack[SP] = stack[SP-1];
            ++SP;
            break;
        case ARITH:
            handle_arithmetic(instruction);
            --SP;
            break;
        case LOGIC:
            handle_logic(instruction);
            break;
        case BSHIFT:
            handle_bit_shift(instruction);
            break;
        case STORER:
            handle_reg_storage(instruction & 0x0FFF);
            break;
        case LOADR:
            handle_reg_load(instruction & 0x0FFF);
            break;
        case IO:
            handle_input_output(instruction);
            break;
        case HALT:
            reg_data[Rhlt] = TRUE;
            break;
    }

    if ((CODE_BASE_INDEX + ++IP) >= MEM_CELL_COUNT/2){
        reg_data[Rhlt] = TRUE;
    }

}

void run_machine(){
    while (MACHINE_IS_RUNNING){
        execute_instruction(code_store[CODE_BASE_INDEX + IP]);
    }
}
