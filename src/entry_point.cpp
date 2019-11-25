
#include <EASTL/string_view.h>
#include <stdio.h>

int main(int argc, char* argv)
{
    eastl::string_view str = "Hello World";
    printf("%s", str.data());

    return 0;
}