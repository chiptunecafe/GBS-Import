#include <string>
#include "GBSImport.h"

char filename[] = "Mega Man Xtreme 2.gbs";

int main(int argc, char** argv)
{
	if(!GBSImport::Import(filename, 8, 5, 0, 1)) goto main_error;

	system("pause");
	return 0;

main_error:

	system("pause");
	return 1;
}