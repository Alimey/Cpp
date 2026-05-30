#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#include <locale.h>

struct Letter {
  int symb;
  int freq;
};

struct Alphabet {
  Letter* letters = (Letter*)malloc(0);
  int sz = 0;
  int cap = 0;
} alph;

int Find(int ch) {
  int i = 0;
  while (i < alph.sz) {
    if (alph.letters[i].symb == ch) {
      return i;
    }
    ++i;
  }
  return -1;
}

struct DecodeLetter {
  int symb;
  char* code;
};

struct DecodeAlphabet {
  DecodeLetter* letters = (DecodeLetter*)malloc(0);
  int sz = 0;
} dalph;

void itoa(int value, char* buffer) {
  int count_dec = 0;
  int dec = 10;
  int copy = value;
  while (copy > 0) {
    ++count_dec;
    copy /= dec;
  }
  for (int i = count_dec - 1; i >= 0; --i) {
    buffer[i] = (value % dec) + '0';
    value /= dec;
  }
  buffer[count_dec] = '\0';
}

void CountFrequences(const char* input_file) {
  FILE* input = fopen(input_file, "rb");
  if (!input) {
    perror("Error opening input file");
    return;
  }
  int ch;
  // Проходимся по файлу
  while ((ch = fgetc(input)) != EOF) {
    int i = Find(ch);
    if (i != -1) {
      alph.letters[i].freq += 1;
      continue;
    }
    if (alph.sz + 1 >= alph.cap) {
      alph.letters = (Letter*) realloc(alph.letters, sizeof(Letter) * (alph.cap * 2 + 1));
      
      alph.cap = 2 * alph.cap + 1;
    }
    alph.letters[alph.sz] = {ch, 1};
    alph.sz += 1;
  }
  fclose(input);
  // Возвращаем alphabet
}

void SortByFreq() {
   for (int i = alph.sz - 1; i >= 0; --i) {
    for (int j = 0; j < i; ++j) {
      if (alph.letters[j].freq < alph.letters[j + 1].freq) {
        Letter l = alph.letters[j];
        alph.letters[j] = alph.letters[j + 1];
        alph.letters[j + 1] = l;
      }
    }
  }
}

void MakeCodes(char** codes, int lborder, int rborder, int deep) {
  if (lborder == rborder) {
    codes[lborder][deep] = '\0';
    return;
  }
  int mborder = lborder + (rborder - lborder) / 2;
  for (int i = lborder; i < mborder + 1; ++i) {
    codes[i][deep] = '0';
  }
  for (int i = mborder + 1; i < rborder + 1; ++i) {
    codes[i][deep] = '1';
  }
  MakeCodes(codes, lborder, mborder, deep + 1);
  MakeCodes(codes, mborder + 1, rborder, deep + 1);
}

void File_to_binary(const char* input_file, char* binary, char** codes) {
  FILE* from = fopen(input_file, "rb");
  if (!from) {
    perror("Error opening file for binary conversion");
    return;
  }
  int ch;
  int it = 0;
  while ((ch = getc(from)) != EOF) {
    char* ch_code = codes[Find(ch)];
    for (int i = 0; ch_code[i] != '\0'; ++i) {
      binary[it] = ch_code[i];
      ++it;
    }
  }
  binary[it] = '\0';
  fclose(from);
}

int GetSizeOfFile(const char* filename) {
  FILE* file = fopen(filename, "rb");
  int ch;
  int size = 0;
  while ((ch = fgetc(file)) != EOF) {
    ++size;
  }
  fclose(file);
  return size;
}

void Encode(char* binary, int size_of_file, const char* output_file, char** codes) {
  FILE* compressed = fopen(output_file, "wb");
  if (!compressed) { 
    perror("Error opening output file for encoding");
    return;
  }
  // Сначала кладем размер файла
  char chsize[256];
  itoa(size_of_file, chsize);
  fputs(chsize, compressed);
  fputc(' ', compressed);
  // Кладем размер алфавита
  char alph_sz[256];
  itoa(alph.sz, alph_sz);
  fputs(alph_sz, compressed);
  fputc(' ', compressed);
  // Кладем алфавит
  for (int i = 0; i < alph.sz; ++i) {
    char letter[256];
    itoa(alph.letters[i].symb, letter);
    fputs(letter, compressed);
    fputc(',', compressed);
    for (int j = 0; codes[i][j] != '\0'; ++j) {
      fputc(codes[i][j], compressed);
    }
    fputc(' ', compressed);
  }
  // Потом кодированный текст
  int count = 0;
  char symb[8]{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  int ch;
  int it = 0;
  while ((ch = binary[it]) != '\0') {
    ++it;
    // Считываем один символ в виде 8 символов 1 и 0
    if (count < 8) {
      symb[count] = ch;
      ++count;
      continue;
    }
    // Кодируем этот символ
    int chr_code = 0;
    int dec = 1;
    for (int i = 0; i < 8; ++i) {
      if (symb[7 - i] == '1') {
        chr_code += dec;
      }
      dec <<= 1;
    }
    fputc(chr_code, compressed); //////?????????
    count = 0;
    it--;
  }
  // Проверяем, кратен ли размер файла 8-ми
  if (count != 0) {
    int chr_code = 0;
    int dec = 1;
    for (int i = 0; i < count; ++i) {
      if (symb[count - 1 - i] == '1') {
        chr_code += dec;
      }
      dec <<= 1;
    }
    fputc(chr_code, compressed);
    count %= 8;
    if (count != 0) {
      char ch_count[256];
      itoa(count, ch_count);
      fputs(ch_count, compressed);
    }
  }
  free(alph.letters);
  free(dalph.letters);
  fclose(compressed);
}

int read_size_from_compressed_file(const char* compressed_name) {
  FILE* compressed = fopen(compressed_name, "rb");
  if (!compressed) { 
    perror("Error opening compressed file to read size");
    return -1;
  }
  int ch;
  char ch_size[256];
  int it = 0;
  while ((ch = fgetc(compressed)) != ' ') {
    ch_size[it] = ch;
    ++it;
  }
  ch_size[it] = '\0';
  int size = atoi(ch_size);
  fclose(compressed);
  return size;
}

int read_size_of_alphabet(const char* compressed_name) {
  FILE* compressed = fopen(compressed_name, "rb");
  if (!compressed) {
    perror("Error opening compressed file to read alphabet size");
    return -1;
  }
  int ch;
  char ch_size[256];
  int it = 0;
  while ((ch = fgetc(compressed)) != ' ') {
    continue;
  }
  while ((ch = fgetc(compressed)) != ' ') {
    ch_size[it] = ch;
    ++it;
  }
  ch_size[it] = '\0';
  int size = atoi(ch_size);
  fclose(compressed);
  return size;
}

void read_alphabet(const char* compressed_name) {
  FILE* compressed = fopen(compressed_name, "rb");
  if (!compressed) {
    perror("Error opening compressed file to read alphabet");
    return;
  }
  int ch;
  int dalph_size = read_size_of_alphabet(compressed_name);
  dalph.sz = dalph_size;
  dalph.letters = (DecodeLetter*)realloc(dalph.letters, sizeof(DecodeLetter) * dalph.sz);
  // Скипаем до второго пробела
  while ((ch = fgetc(compressed)) != ' ') {
    continue;
  }
  while ((ch = fgetc(compressed)) != ' ') {
    continue;
  }
  // Считываем алфавит
  for (int i = 0; i < dalph_size; ++i) {
    // Считываем номер символа до ','
    char symb_code[256];
    int it = 0;
    while ((ch = fgetc(compressed)) != ',') {
      symb_code[it] = ch;
      ++it;
    }
    symb_code[it] = '\0';
    // Переводим его в инт
    int symb = atoi(symb_code);
    // Считываем код символа до ' '
    char* ch_code = (char*)malloc(sizeof(char) * 256);
    it = 0;
    while ((ch = fgetc(compressed)) != ' ') {
      ch_code[it] = ch;
      ++it;
    }
    ch_code[it] = '\0';
    // Создаем букву и добавляем ее в алфавит
    dalph.letters[i] = {symb, ch_code};
  }
  fclose(compressed);
}

void makeBinary(const char* compressed_name, char* binary) {
  int size_of_compressed = GetSizeOfFile(compressed_name);
  int it = 0;
  FILE* compressed = fopen(compressed_name, "rb");
  if (!compressed) {
    perror("Error opening compressed file to make binary");
    return;
  }
  int ch;
  // Скипаем всю ненужную информацию
  for (int i = 0; i < dalph.sz + 2; ++i) {
    while ((ch = fgetc(compressed)) != ' ') {
      ++it;
      continue;
    }
    ++it;
  }
  // Считываем символ из файла и переводим его в двоичное число
  int it_binary = 0;
  while (it < size_of_compressed - 2) {
    ++it;
    ch = fgetc(compressed);
    // Переводим символ в двоичное число
    char code[8]{'0', '0', '0', '0', '0', '0', '0', '0'};
    int i = 0;
    while (ch > 0 && i < 8) {
      code[7 - i] = ch % 2 + '0';
      ch >>= 1;
      ++i;
    }
    // кладем в binary
    for (int j = 0; j < 8; ++j) {
      binary[it_binary] = code[j];
      ++it_binary;
    }
  }
  // Считываем предпоследний символ
  ch = fgetc(compressed);
  // Если за ним лежит еще число, то оно - длина кода последнего символа
  int next = getc(compressed);
  if (next >= '0' && next <= '9') {
    int code_size = next - '0';
    char* code = (char*)malloc(sizeof(char) * code_size);
    for (int i = 0; i < code_size; ++i) {
      code[i] = '0';
    }
    int i = 0;
    while (ch > 0 && i < code_size) {
      code[code_size - 1 - i] = ch % 2 + '0';
      ch >>= 1;
      ++i;
    }
    // кладем в binary
    for (int j = 0; j < code_size; ++j) {
      binary[it_binary] = code[j];
      ++it_binary;
    }
    free(code);
  } else {
    // Если за ним не число, то просто кладем эти два последних символа
    // Переводим символ в двоичное число
    char code[8]{'0', '0', '0', '0', '0', '0', '0', '0'};
    int i = 0;
    while (ch > 0 && i < 8) {
      code[7 - i] = ch % 2 + '0';
      ch >>= 1;
      ++i;
    }
    // кладем в binary
    for (int j = 0; j < 8; ++j) {
      binary[it_binary] = code[j];
      ++it_binary;
    }
    i = 0;
    for (int k = 0; k < 8; ++k) {
      code[i] = '0';
    }

    while (next > 0 && i < 8) {
      code[7 - i] = next % 2 + '0';
      next >>= 1;
      ++i;
    }
    // кладем в binary
    for (int j = 0; j < 8; ++j) {
      binary[it_binary] = code[j];
      ++it_binary;
    }
  }
  binary[it_binary] = '\0';
  fclose(compressed);
}

int find_in_dalphabet(char* code) {
  for (int i = 0; i < dalph.sz; ++i) {
    if (strcmp(dalph.letters[i].code, code) == 0) {
      return dalph.letters[i].symb;
    }
  }
  return -1;
}

void Decode(const char* output_name, char* binary) {
  FILE* output = fopen(output_name, "wb");
  if (!output) {
    perror("Error opening output file for decoding");
    return;
  }
  int it = 0;
  // Читаем код, пока не найдем соответствующий ему в алфавите
  char code[10];
  code[0] = '\0';
  int it_code = 0;
  while (binary[it] != '\0') {
    code[it_code] = binary[it];
    ++it_code;
    code[it_code] = '\0';
    int letter = find_in_dalphabet(code);
    if (letter != -1) {
      fputc(letter, output);
      it_code = 0;
      code[it_code] = '\0';
    }
    ++it;
  }
  for (int i = 0; i < dalph.sz; ++i) {
    free(dalph.letters[i].code);
  }
  free(alph.letters);
  free(dalph.letters);
  fclose(output);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    return 0;
  }
  setlocale(LC_ALL, "");
  const char* input_name = argv[2];
  const char* output_name = argv[3];
  if (strcmp(argv[1], "-c") == 0) {
    char** codes = (char**)malloc(sizeof(char*) * 256);
    for (int i = 0; i < 256; ++i) {
      codes[i] = (char*)malloc(sizeof(char) * 256);
    }
    int size_of_file = GetSizeOfFile(input_name);
    char* binary = (char*)malloc(sizeof(char) * size_of_file * 10);

    CountFrequences(input_name);
    SortByFreq();
    if (alph.sz <= 1) {
      codes[0][0] = '0';
      codes[0][1] = '\0';
    } else {
      MakeCodes(codes, 0, alph.sz - 1, 0);
    }
    File_to_binary(input_name, binary, codes);
    Encode(binary, size_of_file, output_name, codes);

    for (int i = 0; i < 256; ++i) {
      free(codes[i]);
    }
    free(codes);
    free(binary);
  } else {
    int size_of_decompressed_file = read_size_from_compressed_file(input_name);
    int size_of_binary = size_of_decompressed_file * 10;
    char* binary = (char*)malloc(sizeof(char) * size_of_binary);

    read_alphabet(input_name);
    makeBinary(input_name, binary);
    Decode(output_name, binary);
    free(binary);
  }
}