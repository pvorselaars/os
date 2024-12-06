void kernel()
{
        int i = 0;
        char *msg = "Hello, world!";
        char *video = (char*)0xb8000;

        for (char *c = msg; *c != 0; c++) {
               video[i] = *c;
               i += 2;
        }

        while(1);
}
