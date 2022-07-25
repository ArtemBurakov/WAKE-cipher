#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sstream>

using namespace std;

uint32_t s[] = {
    0x726a8f3b,
    0xe69a3b5c,
    0xd3c71fe5,
    0xab3c73d2,
    0x4d3a8eb3,
    0x0396d6e8,
    0x3d4c2f7a,
    0x9ee27cf3
};
uint32_t K[4];
uint32_t S[257];
uint32_t R3[2];
uint32_t R4[2];
uint32_t R5[2];
uint32_t R6[2];
uint32_t auto_key;
string passphrase;
string FILE_REGEX = "^[A-Za-z_.-]+$";

string enterFileName()
{
    string file_name;
    getline(cin, file_name);
    while (file_name.empty() || !regex_match(file_name, regex(FILE_REGEX))) {
        if (file_name.empty())
            cout << "File name can`t be empty.\n";
        else
            cout << "Some of the characters are not allowed. Please, try again!" << endl;
        getline(cin, file_name);
    }
    return file_name;
}

string decToHexa(int n)
{
    char hexaDeciNum[100];
    int i = 0;
    while (n != 0) {
        int temp = 0;
        temp = n % 16;
        if (temp < 10) {
            hexaDeciNum[i] = temp + 48;
            i++;
        }
        else {
            hexaDeciNum[i] = temp + 55;
            i++;
        }
        n = n / 16;
    }
    string result = "";
    for (int j = i - 1; j >= 0; j--)
        result += hexaDeciNum[j];
    return result;
}

uint32_t stringToUint_32t(string ascii)
{
    uint32_t value;
    string hex_string = "";
    for (int i = 0; i < ascii.length(); i++) {
        char ch = ascii[i];
        int tmp = (int)ch;
        string part = decToHexa(tmp);
        hex_string += part;
    }
    istringstream converter(hex_string);
    converter >> hex >> value;
    return value;
}

void writeTextToFile(string text)
{
    string file_name = enterFileName();
    cout << "Writing text to file `" << file_name << "` . . ." << endl;
    ofstream file(file_name);
    file << text;
    file.close();
    cout << "The text was successfully saved to a file!\n" << endl;
}

string getTextFromFile()
{
    string text_string;
    string file_name = enterFileName();
    ifstream file(file_name);
    if (!file)
        throw string("File `" + file_name + "` doesn`t exist!");

    if (file.peek() == ifstream::traits_type::eof())
        throw string("File `" + file_name + "` is empty!");

    cout << "Getting text from file `" << file_name << "` . . ." << endl;
    getline(file, text_string);
    cout << "The text was successfully retrieved from the file!" << endl;

    file.close();
    return text_string;
}

string WAKE(string text)
{
    string output;
    uint32_t temporary[4];
    uint32_t helper = 0xff;
    uint32_t key = auto_key;

    for (int i = 0; i < 4; i++) {
        temporary[i] = key & helper;
        key >>= 8;
    }
    for (int i = 0, j = 3; i < text.length(); i++, j--) {
        if (j < 0) j = 3;
        char ch_text = text[i];
        int tmp_text = int(ch_text);
        int result = tmp_text ^ temporary[j];
        output += result;
    }
    return output;
}

uint32_t M(uint32_t a, uint32_t b, uint32_t* S)
{
    long int m = (a + b) >> 8 ^ S[(a + b) & 255];
    return m;
}

void generateSBlock()
{
    S[0] = K[0];
    S[1] = K[1];
    S[2] = K[2];
    S[3] = K[3];

    uint32_t x;
    for (int i = 4; i < 256; i++) {
        x = S[i - 1] + S[i - 4];
        S[i] = x >> 3 ^ s[x & 7];
    }

    for (int i = 0; i < 23; i++) {
        S[i] += S[i + 89];
    }

    long int X = S[33];
    long int Z = S[59] | 0x01000001;
    Z = Z & 0xFF7FFFFF;
    X = (X & 0xFF7FFFFF) + Z;

    for (int i = 0; i < 256; i++) {
        X = (X & 0xFF7FFFFF) + Z;
        S[i] = S[i] & 0x00FFFFFF ^ X;
    }

    S[256] = S[0];
    X = X & 255;
    uint32_t Temp;
    for (int i = 0; i < 256; i++) {
        Temp = (S[i ^ X] ^ X) & 255;
        S[i] = S[Temp];
        S[X] = S[i + 1];
    }
}

void devideKey()
{
    string temporary;
    int number = 0, n = 4;
    int str_length = passphrase.size();
    int part_size = str_length / n;
    for (int i = 0; i < str_length + 1; i++) {
        if (i != 0 && i % part_size == 0) {
            K[number] = stringToUint_32t(temporary);
            temporary = "";
            number++;
        }
        temporary += passphrase[i];
    }
}

void getPassphrase()
{
    cout << "Enter the file name where the passphrase text is located:" << endl;
    passphrase = getTextFromFile();
    int str_length = passphrase.size();
    if (str_length % 4 != 0 || str_length != 16)
        throw string("Passphrase isn`t correct!");
    devideKey();
}

uint32_t generateAutoKey()
{
    R3[0] = K[0];
    R4[0] = K[1];
    R5[0] = K[2];
    R6[0] = K[3];
    R3[1] = M(R3[0], R6[0], S);
    R4[1] = M(R4[0], R3[1], S);
    R5[1] = M(R5[0], R4[1], S);
    R6[1] = M(R6[0], R5[1], S);
    return R6[1];
}

int main()
{
    try {
        getPassphrase();
        generateSBlock();
        auto_key = generateAutoKey();
        cout << "\nEnter the file name where the plaintext is located:" << endl;
        string plain_text = getTextFromFile();
        string encryptedText = WAKE(plain_text);
        cout << "\nEnter a file name to save the encrypted text:" << endl;
        writeTextToFile(encryptedText);
        cout << "Enter the file name where the encrypted text is located:" << endl;
        string encrypted_text = getTextFromFile();
        string decryptedText = WAKE(encrypted_text);
        cout << "\nEnter a file name to save the decrypted text:" << endl;
        writeTextToFile(decryptedText);
        return 0;
    } catch (const string& exception) {
        cout << exception << endl;
        return EXIT_FAILURE;
    }
}