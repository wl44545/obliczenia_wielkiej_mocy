// Karol Dzialowski 39259

#include <iostream>
#include <tbb/tbb.h>
#include <stdio.h>
#include <algorithm>

using namespace std;

unsigned char*** color;
unsigned char*** filteredColor;
int height;
int width;

int average[9] = {
  1, 1, 1,
  1, 1, 1,
  1, 1, 1,
};

int sobel[9] = {
  1, 2, 1,
  0, 0, 0,
  -1, -2, -1,
};

int sharpen [9] = {
  -1, -1, -1,
  -1, 9, -1,
  -1, -1, -1
};



void load_image() {
  FILE *fp;
  const char *filename = "lenna.ppm";
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  /*reading metadata*/
  fp = fopen(filename, "rb");
  getline(&line, &len, fp); // P6
  getline(&line, &len, fp); // width
  string sizes(line);
  getline(&line, &len, fp); // max component value
  string componentValueString(line);

  /*getting size*/
  sscanf(sizes.c_str(), "%d %d", &width, &height);
  cout << width << ", " << height << endl;

  /*allocation*/
  color = new unsigned char**[width];
  for (int i=0; i<width; i++) {
    color[i] = new unsigned char*[height];
    for (int j=0; j<height; j++) {
      color[i][j] = new unsigned char[3];
    }
  }

  /*reading data*/
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      fread(color[i][j], 1, 3, fp);
    }
  }

  free(line);
  fclose(fp);
}

void filter(int* mask, int maskWidth, int maskSum) {
  // Alokacja kopii obrazu.
  // To bedzie orbaz wynikowy po filtracji.
  filteredColor = new unsigned char**[width];
  for (int i=0; i<width; i++) {
    filteredColor[i] = new unsigned char*[height];
    for (int j=0; j<height; j++) {
      filteredColor[i][j] = new unsigned char[3];
      for (int k=0; k<3; k++) {
        filteredColor[i][j][k] = color[i][j][k];
      }
    }
  }

  // Offset. Jeżeli maska będzie o rozmiarze 3x3 to początek maski będzie oddalony o 1 jednostkę. 
  // Jeżeli maska będzie o rozmiarze 5x5 to początek maski będzie oddalony o 2 jednostki. 
  int offset = maskWidth/2;

  // Pętla po obrazie
  // Wykorzystano parallel_for jak w ksiazce ProTBB str. 45
  // for (int i=0; i<width; i++) {
  tbb::parallel_for(0, width, [=](int i) {
      for (int j=0; j<height; j++) { 
        // Akumulatory dla każdego kanału (RGB)
        int sum_0 = 0;
        int sum_1 = 0;
        int sum_2 = 0;

        // Pętla po masce
        for (int mask_i = 0; mask_i < maskWidth; mask_i++) {
          for (int mask_j = 0; mask_j < maskWidth; mask_j++) {
            // Wyznaczenie indeksu obrazu pod daną pozycją w masce
            int image_i = i + mask_i - offset;
            int image_j = j + mask_j - offset;   

            // Jeżeli maska wystaje poza obraz to uwzględnij te pozycje tak jakby były
            // czarnymi pikselami.
            if (image_i < 0 || image_j < 0 || image_i >= width || image_j >= height) {
              sum_0 += 0;
              sum_1 += 0;
              sum_2 += 0; 
            } 
            // Sumowanie wartości pod danym indeksem maski 
            else {
              // Mnożnik, czyli wartość z maski
              int multiplier = mask[mask_i * maskWidth + mask_j]; 
              sum_0 += multiplier * color[image_i][image_j][0]; 
              sum_1 += multiplier * color[image_i][image_j][1]; 
              sum_2 += multiplier * color[image_i][image_j][2]; 
            } 
          } 
      }

      // Przycinanie wartosci do przedzialu 0,255
      int output_0 = sum_0/maskSum;
      int output_1 = sum_1/maskSum;
      int output_2 = sum_2/maskSum;

      output_0 = std::min(output_0, 255);
      output_0 = std::max(output_0, 0);
      output_1 = std::min(output_1, 255);
      output_1 = std::max(output_1, 0);
      output_2 = std::min(output_2, 255);
      output_2 = std::max(output_2, 0);

      // Ustawianie finalnego piksela po filtracji
      filteredColor[i][j][0] = output_0;
      filteredColor[i][j][1] = output_1;
      filteredColor[i][j][2] = output_2;
      }
  }); 
}

void save_to_file() {
  const int MaxColorComponentValue = 255;
  FILE *fp;
  const char *filename = "output.ppm";
  char *comment = "# ";
  fp = fopen(filename, "wb");
  fprintf(fp, "P6\n %s\n %d\n %d\n %d\n", comment, width, height, MaxColorComponentValue);

  /*write color to the file*/
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      fwrite(filteredColor[i][j], 1, 3, fp);
    }
  }
  fclose(fp);

  /*deallocation*/
  for (int i=0; i<width; i++) {
    for (int j=0; j<height; j++) {
      delete[] color[i][j];
      delete[] filteredColor[i][j];
    }
    delete[] color[i];
    delete[] filteredColor[i];
  }
  delete[] color;
  delete[] filteredColor;
}

int main() {
  load_image();
  tbb::tick_count time_start, time_end;
  time_start = tbb::tick_count::now();

  //filter(sharpen, 3, 1);
  //filter(average, 3, 9);
  filter(sobel, 3, 1);

  time_end = tbb::tick_count::now();
  cout << "Time: " << (time_end - time_start).seconds() << endl;
  save_to_file();
  return 0;
}

