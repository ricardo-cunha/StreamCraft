/*
Copyright 2020, Michael R. Hoopmann, Institute for Systems Biology
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "CSpectrumIdentificationList.h"

using namespace std;

CSpectrumIdentificationList::CSpectrumIdentificationList(){
  numSequencesSearched=0;
}

void CSpectrumIdentificationList::writeOut(FILE* f, int tabs){
  if (id.empty()){
    cerr << "SpectrumIdentificationList::id is required." << endl;
    exit(69);
  }
  if (spectrumIdentificationResult.empty()){
    cerr << "SpectrumIdentificationList::SpectrumIdentificationResult is required." << endl;
    cerr << "SpectrumIdentificationList::id = " << id << endl;
    cerr << "SpectrumIdentificationList::numSequenceSearched = " << numSequencesSearched << endl;
    cerr << "SpectrumIdentificationList::fragmentationTable.size = " << fragmentationTable.size() << endl;
    cerr << "SpectrumIdentificationList::spectrumIdentificationResult.size = " << spectrumIdentificationResult.size() << endl;
    cerr << "SpectrumIdentificationList::cvParam.size = " << cvParam.size() << endl;
    cerr << "SpectrumIdentificationList::userParam.size = " << userParam.size() << endl;
    exit(69);
  }

  int i;
  size_t j;
  for (i = 0; i<tabs; i++) fprintf(f, " ");
  fprintf(f, "<SpectrumIdentificationList id=\"%s\"",&id[0]);
  if(numSequencesSearched>0) fprintf(f," numSequencesSearched=\"%d\"",numSequencesSearched);
  fprintf(f, ">\n");

  int t=tabs;
  if(t>-1) t++;

  for(j=0;j<fragmentationTable.size();j++) fragmentationTable[j].writeOut(f,t);
  for (j = 0; j < spectrumIdentificationResult.size(); j++) spectrumIdentificationResult[j].writeOut(f, t);
  for (j = 0; j<cvParam.size(); j++) cvParam[j].writeOut(f, t);
  for (j = 0; j<userParam.size(); j++) userParam[j].writeOut(f, t);

  for (i = 0; i<tabs; i++) fprintf(f, " ");
  fprintf(f, "</SpectrumIdentificationList>\n");
}
