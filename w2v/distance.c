//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

const long long max_size = 2000;         // max length of strings
const long long max_doc_size = 256;      // max count of words in doc
const long long N = 40;                  // number of closest words that will be shown
const long long max_w = 50;              // max length of vocabulary entries
const long long max_question_len = 10000;
long long words, size;
float *M;
char *vocab;

void str_to_vec(char st1[], float *vec, long long *bi);
float distance(float *vec1, float *vec2);

void str_to_vec(char st1[], float *vec, long long *bi) {
  long long a, b, c, cn;
  char st[256][max_size];
  float len;
//  printf("string: %s\n", st1);
  cn = 0;
  b = 0;
  c = 0;
  while (1) {
    st[cn][b] = st1[c];
    b++;
    c++;
    st[cn][b] = 0;
    if (st1[c] == 0) break;
    if (st1[c] == ' ') {
      cn++;
      b = 0;
      c++;
    }
  }
  cn++;
  
  for (a = 0; a < max_doc_size; a++) bi[a] = 0;
  for (a = 0; a < cn; a++) {
    for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
    if (b == words) b = -1;
    bi[a] = b;
//    printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
    if (b == -1) {
//      printf("Out of dictionary word!\n");
      break;
    }
  }
  
//  if (b == -1) return;
//  printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
  for (a = 0; a < size; a++) vec[a] = 0;
  for (b = 0; b < cn; b++) {
    if (bi[b] == -1) return;
    for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
  }
  
  len = 0;
  for (a = 0; a < size; a++) len += vec[a] * vec[a];
  len = sqrt(len);
  for (a = 0; a < size; a++) vec[a] /= len;
  
  return;
}

//float distance(float *vec1, float *vec2) {
//  float dist;
//  
//  dist = 0;
//  for (int a = 0; a < size; a++) dist += vec1[a] * vec2[a];
//  
//  return dist;
//}

float distance(float *A, float *B) {
  double dot = 0.0, denom_a = 0.0, denom_b = 0.0 ;
  for(unsigned int i = 0u; i < size; ++i) {
    dot += A[i] * B[i] ;
    denom_a += A[i] * A[i] ;
    denom_b += B[i] * B[i] ;
  }
  return dot / (sqrt(denom_a) * sqrt(denom_b)) ;
}

const char* getfield(char* line, int num) {
  const char* token;
  char s[2] = ",";
  token = strtok(line, s);
  
  int i = 0;
  while( token != NULL ) {
    if (i == num) {
      return token;
    }
    token = strtok(NULL, s);
    i++;
  }
  
  return NULL;
}

int main(int argc, char **argv) {
  FILE *f;
  char *bestw[N];
  float dist, len, bestd[N], vec[max_size], ans[max_size];
  long long a, b, c, cn, bi[max_doc_size], abi[max_doc_size];
  int first_ans_i;

  // f = fopen("/Users/joshsilverman/Dropbox/Apps/salsify-pres/data/freebase-vectors-skipgram1000.bin", "rb");
  f = fopen("/Users/joshsilverman/Dropbox/Apps/salsify-pres/data/GoogleNews-vectors-negative300.bin", "rb");
  if (f == NULL) {
    printf("Input file not found\n");
    return -1;
  }
  // printf("\na");

  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
    return -1;
  }
  // printf("\nb");
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);
  for (a = 0; a < N; a++) bestd[a] = 0;
  for (a = 0; a < N; a++) bestw[a][0] = 0;

  // printf("\nc");
  
  FILE* stream;
  stream = fopen("/Users/joshsilverman/Dropbox/Apps/salsify-pres/data/validation_set.csv", "rt");
  first_ans_i = 2;
  // stream = fopen("/Users/joshsilverman/Dropbox/Apps/cosi101a/cosi101a/data/training_set_tfidf.csv", "r");
  // first_ans_i = 3;
  
  char line[max_question_len];
  fgets(line, max_question_len, stream);
  while (fgets(line, max_question_len, stream)) {
    size_t len = strlen(line);
    if (len > 0 && line[len-1] == '\n') {
      line[--len] = '\0';
    }
    
    char* tmp = strdup(line);
    char* field = getfield(tmp, 1);
    //  printf("\nQuestion: %s\n", field);
    str_to_vec(field, vec, bi);

    tmp = strdup(line);
    field = getfield(tmp, 0);
    printf("\n");
    
    for (int i = first_ans_i; i <= first_ans_i + 3; i++) {
      tmp = strdup(line);
      field = getfield(tmp, i);
      // printf("%i: %s,", i, field);
      
      str_to_vec(field, ans, abi);
      
      dist = distance(vec, ans);
      printf("\t %f", dist);
    }

    free(tmp);
  }
  printf("\n\n");
}
