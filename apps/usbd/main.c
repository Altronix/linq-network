#include "linq_usbd.h"
#include "log.h"

static volatile int running = 1;

void
ctrlc(int dummy)
{
    running = 0;
}

void
sighup(int dummy)
{}

int
main(int argc, char* argv[])
{
    return 0;
}
