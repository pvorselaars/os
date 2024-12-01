SECTIONS {
  /* BIOS loads bootsector at 0x0000:0x7c00 */
  . = 0x7c00;

  .text : {
      *(.text)
  } = 0x0

}
