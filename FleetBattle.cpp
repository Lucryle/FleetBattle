#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

const string SHIP_FILE = "1stships.txt";
const string AIM_FILE = "2ndaim.txt";

bool isValidCoord(const string& coord) {
    if (coord.length() < 2 || coord.length() > 3) 
        return false;

	if (!isalpha(coord[0]) || !isdigit(coord[1])) // 9A gibi bir koordinat girildiginde abort'u onlemek icin
		return false;
	if (coord.length() == 3 && !isdigit(coord[2])) // 10A gibi bir koordinat girildiginde abort'u onlemek icin
		return false;

    char row = toupper(coord[0]);
    int col = stoi(coord.substr(1));

    return row >= 'A' && row <= 'J' && col >= 1 && col <= 10;
}

char getValueFromBoard(const string& filename, const string& coord) {
    int row = toupper(coord[0]) - 'A';
    int col = stoi(coord.substr(1)) - 1;

	// Her satirda 12 karakter var (10 koordinat + 2 satir sonu karakteri)
    streampos pos = row * 12 + col;

    fstream file(filename, ios::in | ios::binary);
    if (file.is_open()) 
    {
        file.seekg(pos);
        char ch;
        file.get(ch);
        file.close();
        return ch;
    }
    else 
    {
        cerr << "Dosya acilamadi." << endl;
        return '?';
    }
}


void updateBoard(const string& filename, const string& coord, char value) {
    int row = toupper(coord[0]) - 'A';
    int col = stoi(coord.substr(1)) - 1;

    streampos pos = row * 12 + col;

    fstream file(filename, ios::in | ios::out | ios::binary);
    if (file.is_open()) 
    {
        file.seekp(pos);
        file.put(value);
        file.close();
    }
    else 
    {
        cerr << "Dosya acilamadi." << endl;
        return;
    }
}

void createEmptyBoardFile(const string& filename) {
    ofstream file(filename, ios::binary);
    for (int i = 0; i < 10; ++i) {
        file << string(10, '-') << "\r\n";
    }
}

// Bir koordinatin etrafindaki gecerli tum hucreleri kontrol eder
bool checkSurroundingCells(const string& coord) {
    int row = toupper(coord[0]) - 'A';
    int col = stoi(coord.substr(1)) - 1;
    
    for(int i = -1; i <= 1; i++) 
    {
        for(int j = -1; j <= 1; j++)
        {
            if(i == 0 && j == 0) 
                continue;
            
            int newRow = row + i;
            int newCol = col + j;
            
            if(newRow < 0 || newRow > 9 || newCol < 0 || newCol > 9) 
                continue;
            
            string checkCoord = string(1, 'A' + newRow) + to_string(newCol + 1);
            char value = getValueFromBoard(SHIP_FILE, checkCoord);
            
            if(value != '-')
                return false;
        }
    }
    return true;
}

// Koordinatlarin birbirine bagli olup olmadigini kontrol eder 
bool areCoordinatesConnected(const vector<string>& coords) {
	if (coords.size() == 1) 
        return true;

	char firstRow = toupper(coords[0][0]);
	int firstCol = stoi(coords[0].substr(1));

	for (int i = 1; i < coords.size(); i++) 
    {
		char currentRow = toupper(coords[i][0]);
		int currentCol = stoi(coords[i].substr(1));
		if (!(firstRow == currentRow || firstCol == currentCol))
			return false;
	}
	return true;
}

void placeShips() {
    char shipTypes[] = { 'A', 'B', 'C', 'D', 'E' };
    int shipLengths[] = { 5, 4, 3, 2, 1 };
    int shipCounts[] = { 1, 1, 2, 2, 3 };
    int placedCounts[] = { 0, 0, 0, 0, 0 };

    while (true) 
    {
        cout << "Koyulmasi gereken gemiler:" << endl;
        bool anyLeft = false;
        for (int i = 0; i < 5; ++i) 
        {
            if (placedCounts[i] < shipCounts[i])
            {
                cout << shipTypes[i] << " (uzunluk: " << shipLengths[i] << ", kalan: " << shipCounts[i] - placedCounts[i] << ")" << endl;
                anyLeft = true;
            }
        }
        if (!anyLeft) break;

        cout << "Hangi gemiyi koymak istersin? ";
        char ship;
        cin >> ship;
        ship = toupper(ship);

        int idx = -1;
        for (int i = 0; i < 5; i++) 
        {
            if (shipTypes[i] == ship) 
            {
                idx = i;
                break;
            }
        }

        if (idx == -1 || placedCounts[idx] >= shipCounts[idx]) 
        {
            cout << "Gecersiz veya zaten koyulmus gemi turu." << endl;
            continue;
        }

        vector<string> coords;
        cout << ship << " icin koordinatlari TEKTEK gir: " << shipLengths[idx] << " tane koordinat gireceksin." << ":" << endl;
        while (coords.size() < shipLengths[idx]) 
        {
            string coord;
            cin >> coord;
            if (!isValidCoord(coord)) {
                cout << "gecersiz koordinat, en bastan duzgun koordinat gir" << endl;
                coords.clear(); // Tum koordinatlari sifirla
                continue;
            }
            
            // Koordinatin bos olup olmadigini kontrol et
            char current = getValueFromBoard(SHIP_FILE, coord);
            if (current != '-') {
                cout << coord << " -> bu koordinat zaten dolu: " << current << " | En bastan farkli ve gecerli bir koordinat gir." << endl;
                coords.clear();
                continue;
            }
            
            // Etrafindaki hucreleri kontrol et
            if (!checkSurroundingCells(coord)) {
                cout << coord << " -> bu koordinat baska bir geminin yanina koyulamaz. En bastan farkli ve gecerli bir koordinat gir." << endl;
                coords.clear();
                continue;
            }
            
            coords.push_back(coord);
            
            // Eger tum koordinatlar girildiyse, birbirine bagli olup olmadigini kontrol et
            if (coords.size() == shipLengths[idx]) 
            {
                if (!areCoordinatesConnected(coords)) 
                {
                    cout << "Koordinatlar birbirine bagli olmali (yatay veya dikey). tum koordinatlari bastan gir." << endl;
                    coords.clear();
                    continue;
                }
            }
        }

		cout << "Gemi " << ship << " yerlestirildi: " << coords.size() << " koordinat girildi." << endl;
		cout << "Gemi koordinatlari: " << endl;

		for (const string& c : coords) {
			cout << c << " ";
		}
		cout << endl;

        for (const string& c : coords) {
            updateBoard(SHIP_FILE, c, ship);
        }
        placedCounts[idx]++;
    }
}

void makeShot() {
	cout << "Atis yapilacak koordinati gir (A1, B2, C3, ...): ";
    string coord;
    cin >> coord;

    if (!isValidCoord(coord)) {
        cout << "Gecersiz koordinat." << endl;
        return;
    }

    char target = getValueFromBoard(SHIP_FILE, coord);
    char already = getValueFromBoard(AIM_FILE, coord);

    if (already != '-') {
        cout << "Buraya zaten vurdun!" << endl;
        return;
    }

    if (target != '-' && target != 'X') {
        cout << "VURDUN!" << endl;
        updateBoard(AIM_FILE, coord, 'O');
        updateBoard(SHIP_FILE, coord, 'O');
    }
    else {
        cout << "KARAVANA." << endl;
        updateBoard(AIM_FILE, coord, 'X');
        updateBoard(SHIP_FILE, coord, 'X');
    }
}

bool checkIfOngoingGame() {
	fstream file(SHIP_FILE, ios::in | ios::binary);
	int count = 0;
    char ch;
	while (file.get(ch)) {
		if (ch == '-' || ch == 'X')
			count++;
	}

    return count == 78; // Tum gemiler yerlestirildiginde, 78 bos hucre kalir (10x10 - 5 - 4 - 3 * 2 - 2 * 2 - 1 * 3)
}

bool checkIfGameOver() {
	fstream file(AIM_FILE, ios::in | ios::binary);
	int count = 0;
	char ch;
	while (file.get(ch)) {
		if (ch == 'O')
			count++;
	}

	return count >= 22; // 22 isabetli atis yapilmis ise, oyun bitmis demektir (5 + 4 + 3 * 2 + 2 * 2 + 1 * 3 = 22)
}

int main() {
	if (checkIfOngoingGame()) 
    {
		cout << "Yarim kalmis bir oyun bulundu, Devam etmek ister misin? Y/N" << endl;
		string choice;
		cin >> choice;
		if (choice[0] == 'Y') 
            cout << "Oyun devam ediyor..." << endl;
        else 
        {
            cout << "Yeni bir oyun baslatiliyor..." << endl;
            createEmptyBoardFile(SHIP_FILE);
            createEmptyBoardFile(AIM_FILE);
            cout << "=== Gemi yerlestirme  ===" << endl;
            placeShips();
        }
	}
	else 
    {
		cout << "Yeni bir oyun baslatiliyor..." << endl;
		createEmptyBoardFile(SHIP_FILE);
		createEmptyBoardFile(AIM_FILE);
		cout << "=== Gemi yerlestir! ===" << endl;
		placeShips();
	}

	cout << "\n\nTum gemiler yerlestirildi. Simdi atis yapabilirsin!" << endl;

    while (true) 
    {
        cout << "\n=== Atis yap! ===" << endl;
        makeShot();
		if (checkIfGameOver()) {
			cout << "\n\nOyun bitti! Tum gemiler batirildi." << endl;
			cout << "Cikmak icin herhangi bir tusa bas." << endl;

			cin.ignore();
			cin.get(); // Kullanici herhangi bir tusa basana kadar bekle

			createEmptyBoardFile(SHIP_FILE);
			createEmptyBoardFile(AIM_FILE);
  			break;
		}
    }

    return 0;
}
