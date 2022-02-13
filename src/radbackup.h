/*
 *  radBackup.h
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef radBackup_H
#define radBackup_H

#include "radimgfunc.h"
#include <QDir>
#include <QDirIterator>

// Math
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>

class radBackup
{
private:

	void WriteSplitFeatures(const char *, DoublePointArray2D &, DoublePointArray2D &, vector< float > &, 
		vector< float > &, vector< float > &, vector< float > &, vector< pair<unsigned int, unsigned int> > &,
		DoublePointArray2D &, DoublePointArray2D &);

	bool ReadTxtFile(const char *, vector<string> &, vector<string> &);

	template <class T>
	void ExtractNumsFromString(string &, vector<T> &, int data_type);
	
	void ReadSplitFeatures(const char *, DoublePointArray2D &, DoublePointArray2D &, vector< float > &, 
		vector< float > &, vector< float > &, vector< float > &, vector< pair<unsigned int, unsigned int> > &,
		DoublePointArray2D &, DoublePointArray2D &);
	bool IsDiretoryExisted(pair<string, string> &);
	bool IsRecordExisted(string &, string &);
	int GetDirectoryIndex(pair<string, string> &);
	int GetMaximumDirectoryIndex(pair<string, string> &);
	void ExtractSplitParameters(string &, ConeDetectionParameters &);

	string BackupDir;

public:
	
	radBackup();
	~radBackup();

	void WriteBackupResults(SplitImageInformation & split_info,
		ConeDetectionParameters & detection_paras, int features_only=0);
	bool ReadSplitBackup(SplitImageInformation &split_infor, ConeDetectionParameters &detection_paras,
		int skip_features=0);
	inline void SetBackupDir(string str) {BackupDir.assign(str);}
	bool RemoveDir(const QString &dirName);
};

#endif // radBackup_H

