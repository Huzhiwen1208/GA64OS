.section .start.text
.globl _start

_start:
    mrs     x8, mpidr_el1 // Read the MPIDR_EL1 register to get the CPU ID
    and     x8, x8, #0x3  // Mask to get the CPU ID
    cbz     x8, 2f        // If CPU ID is 0, jump to the main entry point
1:
    wfe                 // Wait for event (WFE) - put the CPU to sleep until an event occurs
    b       1b          // Loop back to wait for the next event
2:
    ldr     x1, =_start     // Load the address of _start into x1
    mov     sp, x1

// clear bss
    ldr     x1, =__bss_start    // Load the start address of the .bss section
    ldr     w2, =__bss_size     // Load the size of the .bss section
3:
    cbz     w2, 4f              // If the size is zero, jump to the next step
    str     xzr, [x1], #8       // Store zero to the current address and increment the pointer
    sub     w2, w2, #1          // w2--
    cbnz    w2, 3b              // If w2 is not zero, loop back to clear the next word
4:
    bl      main
    b       1b                  // Loop indefinitely after main returns
