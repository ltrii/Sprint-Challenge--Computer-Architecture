#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATA_LEN 6

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char location)
{
  return cpu->ram[location];
}

void cpu_ram_write(struct cpu *cpu, unsigned char location, unsigned char value)
{
  cpu->ram[location] = value;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *file)
{
  FILE *fptr;
  char line[1024];
  unsigned char location = 0;
  fptr = fopen(file, "r");

  if (fptr == NULL)
  {
    fprintf(stderr, "Failed to open! %s\n", file);
    exit(2);
  }

  while (fgets(line, sizeof(line), fptr) != NULL)
  {
    char *endptr;
    unsigned char binstruc;

    binstruc = strtoul(line, &endptr, 2);

    if (endptr == line)
    {
      continue;
    }

    cpu_ram_write(cpu, location++, binstruc);
  }

  fclose(fptr);
}

/**
 * ALU
 */

void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  unsigned char valueA = cpu->registers[regA];
  unsigned char valueB = cpu->registers[regB];

  switch (op)
  {
  case ALU_MUL:
    cpu->registers[regA] = valueA * valueB;
    break;
  case ALU_ADD:
    cpu->registers[regA] = valueA + valueB;
    break;
  case ALU_CMP:
    goto aLabel;
    aLabel: ;
    unsigned char flag;
    if (valueA == valueB)
    {
      flag = 0b00000001 & cpu->ram[cpu->pc];
    }
    else if (valueA > valueB)
    {
      flag = 0b00000010 & cpu->ram[cpu->pc];
    }
    else if (valueA < valueB)
    {
      flag = 0b00000100 & cpu->ram[cpu->pc];
    }
    else
    {
      flag = 0b00000000;
    }
    cpu->fl = flag;
    break;

    // TODO: implement more ALU ops
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  unsigned char ir;
  unsigned char opA;
  unsigned char opB;

  while (running)
  {
  
    ir = cpu_ram_read(cpu, cpu->pc);
   
    opA = cpu_ram_read(cpu, cpu->pc + 1);
    opB = cpu_ram_read(cpu, cpu->pc + 2);
  
    switch (ir)
    {
    case LDI:
      cpu->registers[opA] = opB;
      break;
    case PRN:
      printf("%d\n", cpu->registers[opA]);
      break;
    case MUL:
      alu(cpu, ALU_MUL, opA, opB);
      break;
    case ADD:
      alu(cpu, ALU_ADD, opA, opB);
      break;
    case POP:
      cpu->registers[opA] = cpu->ram[cpu->registers[7]];
      cpu->registers[7]++;
      break;
    case PUSH:
      cpu->registers[7]--;
      cpu_ram_write(cpu, cpu->registers[7], cpu->registers[opA]);
      break;
    case CALL:
      cpu->registers[7]--;
      cpu_ram_write(cpu, cpu->registers[7], cpu->pc + 2);
      cpu->pc = cpu->registers[opA];
      break;
    case RET:
      cpu->pc = cpu->ram[cpu->registers[7]];
      cpu->registers[7]++;
      break;
    case HLT:
      running = 0;
      break;
    case CMP:
      alu(cpu, ALU_CMP, opA, opB);
      break;
    case JMP:
      cpu->pc = cpu->registers[opA];
      break;
    case JNE:
      if ((cpu->fl & 0b00000001) == 0)
      {
        cpu->pc = cpu->registers[opA];
      }
      else
      {
        cpu->pc += 2;
      }
    default:
      printf("unexpected instruction 0x%02X at 0x%02X\n", ir, cpu->pc);
      exit(1);
      break;
    }

    if ((0b11000000 & ir) >> 6 == 1 && ir != CALL && ir != RET)
    {
      cpu->pc += 2;
    }
    else if ((0b11000000 & ir) >> 6 == 2)
    {
      cpu->pc += 3;
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the pc and other special registers

  memset(cpu->registers, 0, 7);
  cpu->registers[7] = 0xf4;

  cpu->pc = 0;

  cpu->fl = 0;

  memset(cpu->ram, 0, sizeof(cpu->ram));
}
