extern long read( int fd, char* data, long maxlen);
extern long write( int fd, const char* data, long len);
extern __attribute__((noreturn)) void exit( long status);

int main( int argc, char* argv[])
{
char data[1024];

return write( 1, data, read( 0, data, sizeof(data)));
}
