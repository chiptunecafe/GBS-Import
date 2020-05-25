namespace GBSImport
{
	bool Scan(char* filename);
	bool Import(char* filename, int track, int seconds, bool alltracks, bool nosilence);
	char* GetError();
}