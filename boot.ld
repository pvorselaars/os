SECTIONS {
  /* BIOS loads bootsector at 0x0000:0x7c00 */
  . = 0x7c00;

  .text : {
      .start = . ;

      *(.text);

      .end   = . ;

      /* add boot signature */
      . += 0x1fe - .end - .start;
      BYTE(0x55);
      BYTE(0xAA);

      LONG(0x0b501e7e);
  } = 0x0

}
