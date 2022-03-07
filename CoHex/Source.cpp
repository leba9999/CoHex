#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>
#include <ctime>
namespace fs = std::experimental::filesystem::v1;

std::vector<std::string> files;

/**
	Muuntaa tiedoston tyypin .wav muotoon poistamalla 12 tavua(bytes) tiedostosta.
	Tarkennus: avaa tiedoston, poistaa muistiin ladatusta tiedostosta ensimmäistä 12 tavua(bytes), kirjoittaa
	uuden tiedoston samalla nimellä mutta uudella päättellä .wav, lopuksi metodi kirjoittaa uuden tiedoston ja poistaa alkuperäisen tiedoston kansiosta
	@param filename tiedoston polku ja nimi
*/
void convert(std::string filename) {
	// lukija
	std::ifstream myFile;
	try {
		// yritetään avata tiedosto binääri muodossa, jotta voidaan sitten poistaa tavuja
		myFile.open(filename, std::ios::binary | std::ios::ate);
		// lue tiedosto result vektoriin
		std::ifstream::pos_type pos = myFile.tellg();
		std::cout << "[Reading file]: " + filename << std::endl;
		std::vector<char>  result(pos);
		myFile.seekg(0, std::ios::beg);
		myFile.read(&result[0], pos);
		myFile.close();

		std::string wave = "";
		int test = 0;
		bool overWrite = false;
		// etsitään löytyykö WAVE sanaa tavuista, jos löytyy WAVE alle 13 tavun on tiedosto jo wav muodossa
		// jos fssm löytyy tiedoston tavuista on muunnettava tiedosto uuteen muotoon
		for (size_t i = 0; i < 32; i++)
		{
			wave += result[i];
			test++;
			if (wave.compare("WAVE") == 0 && i < 13) {
				overWrite = false;
			}
			else if (wave.compare("fssm") == 0) {
				overWrite = true;
			}
			// etsitään vain 4 kirjaimisia sanoja tiedostosta
			if (test > 3) {
				wave = "";
				test = 0;
			}
		}
		// jos voidaan ylikirjoittaa tiedosto kirjoitetaan se uudelleen uudella tavu järjestelmällä
		if (overWrite) {
			// haetaan tiedoston nimi ilman loppu tunnistetta (esim:.png)
			size_t lastindex = filename.find_last_of(".");
			// puhdas nimi ja polku ilman lopputunnistetta 
			std::string rawname = filename.substr(0, lastindex);
			std::cout << "[Converting file]: " + filename << std::endl;
			// poistaa 12 tavua tiedoston alusta
			result.erase(result.begin(), result.begin() + 12);
			std::cout << "[Saving to file]: " + (rawname + ".wav") << std::endl;
			// kirjoitetaan uusi tiedosto 
			std::ofstream  newFile(rawname + ".wav", std::ios::binary | std::ios::ate);
			newFile.write(result.data(), result.size());
			newFile.close();
			// poistetaan vanha tiedosto
			remove(filename.data());
		}
		else {
			std::cerr << "[Error: Cannot convert file]: " + filename << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << "[Error]: " << e.what() << std::endl;
	}
}
/**
	Rekursiivinen metodi joka kutsuu itse itseään aina kun löytyy "kansio" eli tiedosto jota epäillään kansioksi.
	Jos metodi löytää pelkästään tiedoston työnnetään files vektoriin tiedoston polku
	@param path polku mistä etsitään kansioita
*/
void findSubFolders(std::string path) {
	if (path.size() > 0) {
		// Etsii kaikki tiedostot tiedostopolusta (path)
		for (auto & p : fs::directory_iterator(path))
		{
			std::stringstream ss;
			// siirretään stringstreamiin löydetyn tiedoston tiedostopolku
			ss << p;
			// jos piste löytyy tiedoston polusta on todennäköisesti kyseessä tiedosto, muuten oletetaan että on kansio
			if (ss.str().find('.') == std::string::npos) {
				// jos ei löytynyt pistettä on kyseessä todennäköisesti kansio ja mennään tutkimaan sitä
				findSubFolders(ss.str());
			}
			else {
				// jos löyty piste lisätään files vektoriin tiedostonpolku (sisältää tiedosto nimen)
				files.push_back(ss.str());
			}
		}
	}
}

int main() {
	while (true)
	{
		// Tyhjennetään files vektori jos edellisellä keralla asetettiin sinne jotain
		files.clear();
		std::string path = "";
		std::cout << "enter path: ";
		// heataan käyttäjän syöttämä polku
		std::getline(std::cin, path);
		// muunnetaan kaikki takaperin olevat kauttamerkit
		for (size_t i = 0; i < path.size(); i++){
			if (path[i] == '\\') {
				path[i] = '/';
			}
		}
		// varmistetaan että käyttäjä oli syöttänyt jotain
		if (path.size() > 0) {
			// etsitään kaikki tiedostot jopa alikansioista
			findSubFolders(path);
			std::cout << "Files: " << files.size() << std::endl;
			int filereport = 1000;
			// muunnetaan tiedostot files vektorista
			for (size_t i = 0; i < files.size(); i++)
			{
				convert(files[i]);
				// päivitetään joka 1000 tiedoston kohdalla käytäjälle missä tiedostossa mennään
				if (filereport <= i) {
					filereport += 1000;
					std::cout << "File " << i << ": " << files[i] << std::endl;
				}
			}
		}
	}
	return 0;
}