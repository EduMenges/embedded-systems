#include "hash.h"

static uint64_t dummy[] = {0, 1, 2, 3, 4, 5, 6, 7};

int main()
{
    hash(dummy, sizeof(dummy));
}
