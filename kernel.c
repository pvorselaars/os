void kernel()
{
        char *video = (char*)0xb8000;
        video[0]= 'K';
        video[1]= 0x0f;
        video[2]= 'E';
        video[3]= 0x0f;
        video[4]= 'R';
        video[5]= 0x0f;
        video[6]= 'N';
        video[7]= 0x0f;
        video[8]= 'E';
        video[9]= 0x0f;
        video[10]= 'L';
        video[11]= 0x0f;

        while(1);
}
