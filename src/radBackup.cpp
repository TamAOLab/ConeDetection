/*
 *  radBackup.cpp
 *  
 *
 *  Created by Jianfei Liu on 9/19/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "radbackup.h"
#include <QDebug>
#include "QuickView.h"
#include <cfloat>
#include <limits>
#include <QFileInfo>
#include <ctime>
#include <iostream>
#include <string>

radBackup::radBackup()
{
	
}

radBackup::~radBackup()
{ 
}

bool radBackup::ReadTxtFile(const char *filename, vector<string> & str_list, vector<string> & str_list1)
{
	string line;
	ifstream myfile(filename);

	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			if (!line.empty() && line[0] != '#' && line.find("Created time") == string::npos)
				str_list.push_back(line);
			else if (!line.empty() && line[0] == '#')
				str_list1.push_back(line);

		}
		myfile.close();
		return true;
	}
	else
		return false;
}

template <class T>
void radBackup::ExtractNumsFromString(string &record, vector<T> &arr, int data_type)
{
	size_t found, found1;
    string value_str;
    found = record.find(',');
    found1 = 0;
    
    while (found != string::npos) 
	{
        if (found1 == 0) 
		{
			value_str.assign(record.begin()+found1, record.begin()+found);
        }
        else
            value_str.assign(record.begin()+found1+1, record.begin()+found);
        
		if (data_type == 0) //int/short
			arr.push_back(atoi(value_str.c_str()));
		else if (data_type == 1) //float/double
			arr.push_back(atof(value_str.c_str()));
		else if (data_type == 2) //unsigned int
		{
			arr.push_back(atol(value_str.c_str()));
		}

		found1 = found;
		found = record.find(',', found+1);
    }
    
    value_str.assign(record.begin()+found1+1, record.end());
    arr.push_back(atof(value_str.c_str()));
}

void radBackup::ReadSplitBackup(SplitImageInformation & split_infor, ConeDetectionParameters & detection_paras, int skip_features)
{

	//get all sub-directories
	QDirIterator it(BackupDir.c_str(), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	vector<string> str_list, str_list1;
	string img_file_path;
	while (it.hasNext()) 
	{
		str_list.clear();
		string filename = it.next().toStdString() + "/" + split_infor.split_files.second + ".txt";

		if (ReadTxtFile(filename.c_str(), str_list, str_list1) && str_list.size() >= 3)
		{
			if (str_list[0].compare(split_infor.split_files.first) != 0)
				continue;

			if (!skip_features)
				ReadSplitFeatures(str_list[2].c_str(), split_infor.split_left_detections, split_infor.split_right_detections, 
					split_infor.split_left_detection_scales, split_infor.split_right_detection_scales, 
					split_infor.split_left_detection_weights, split_infor.split_right_detection_weights, 
					split_infor.split_detection_links, split_infor.split_final_detections, split_infor.split_edited_detections);

			//get detection paras
			for (unsigned int i=0; i<str_list1.size(); i++)
			{
				//load all kinds of settings
				ExtractSplitParameters(str_list1[i], detection_paras);
			}

			if (str_list.size() > 4)
			{
				size_t pos;
				pos = str_list[3].find(",");
				string flag;
				flag.assign(str_list[3].begin() + pos + 1, str_list[3].end());
				img_file_path.assign(str_list[3].begin(), str_list[3].begin() + pos);

				pos = str_list[4].find(",");
				flag.assign(str_list[4].begin() + pos + 1, str_list[4].end());
				img_file_path.assign(str_list[4].begin(), str_list[4].begin() + pos);
			}
		}
	}
}

bool radBackup::RemoveDir(const QString & dirName)
{
	bool result = true;
    QDir dir(dirName);
 
    if (dir.exists(dirName)) 
	{
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) 
		{
            if (info.isDir()) 
			{
                result = RemoveDir(info.absoluteFilePath());
            }
            else 
			{
                result = QFile::remove(info.absoluteFilePath());
            }
 
            if (!result) 
			{
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
 
    return result;
}

void radBackup::WriteSplitFeatures(const char * filename, DoublePointArray2D & left_detections, DoublePointArray2D & right_detections, 
								   vector< float > & left_scales, vector< float > &right_scales, vector< float > &left_weights, 
								   vector< float > & right_weights, vector< pair<unsigned int, unsigned int> > & detection_links,
								   DoublePointArray2D & final_detections, DoublePointArray2D & edited_detections)
{
	ofstream myfile;

	myfile.open(filename);
	for (unsigned int i=0; i<left_detections.size(); i++)
	{
		myfile << "left_detections:" << left_detections[i][0] << "," << left_detections[i][1] << "," 
			<< left_scales[i] << "," << left_weights[i] << std::endl;
	}
	for (unsigned int i=0; i<right_detections.size(); i++)
	{
		myfile << "right_detections:" << right_detections[i][0] << "," << right_detections[i][1] << "," 
			<< right_scales[i] << "," << right_weights[i] << std::endl;
	}
	for (unsigned int i=0; i<detection_links.size(); i++)
	{
		myfile << "detection_links:" << detection_links[i].first << "," << detection_links[i].second << std::endl;
	}
	for (unsigned int i = 0; i < final_detections.size(); i++)
	{
		myfile << "final_detections:" << final_detections[i][0] << "," << final_detections[i][1] << std::endl;
	}
	for (unsigned int i = 0; i < edited_detections.size(); i++)
	{
		myfile << "edited_detections:" << edited_detections[i][0] << "," << edited_detections[i][1] << std::endl;
	}
	myfile.close();
}

void radBackup::ExtractSplitParameters(string & input_str, ConeDetectionParameters & detection_paras)
{
	size_t pos;
	pos = input_str.find(":");
	if (pos == string::npos)
		return;

	string str1, str2;
	str1.assign(input_str.begin(), input_str.begin()+pos);
	str2.assign(input_str.begin()+pos+1, input_str.end());

	if (str1.find("VotingRadius") != string::npos)
		detection_paras.VotingRadius = atoi(str2.c_str());
	else if (str1.find("GradientMagnitudeThreshold") != string::npos)
		detection_paras.GradientMagnitudeThreshold = atof(str2.c_str());
	else if (str1.find("Scale") != string::npos)
		detection_paras.Scale = atof(str2.c_str());
	else if (str1.find("DimConeDetectionFlag") != string::npos)
		detection_paras.DimConeDetectionFlag = atoi(str2.c_str());
	else if (str1.find("LOGResponse") != string::npos)
		detection_paras.LOGResponse = atof(str2.c_str());
}

void radBackup::ReadSplitFeatures(const char *filename, DoublePointArray2D & left_detections, DoublePointArray2D & right_detections, 
								   vector< float > & left_scales, vector< float > &right_scales, vector< float > &left_weights, 
								   vector< float > & right_weights, vector< pair<unsigned int, unsigned int> > & detection_links,
								   DoublePointArray2D & final_detections, DoublePointArray2D & edited_detections)
{
	string line, tmp_str, label_str;
	ifstream myfile(filename);
	vector<float> tmp_array;
	vector<unsigned int> tmp_array1;

	left_detections.clear();
	right_detections.clear();
	left_scales.clear();
	right_scales.clear();
	left_weights.clear();
	right_weights.clear();
	detection_links.clear();
	final_detections.clear();
	edited_detections.clear();

	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			if (!line.empty() && line[0] != '#')
			{
				size_t pos = line.find(":");
				if (pos == string::npos)
					continue;

				label_str.assign(line.begin(), line.begin()+pos);
				tmp_str.assign(line.begin()+pos+1, line.end());

				DoublePointType2D ft;
				float scale;
				float weight;

				tmp_array.clear();
				tmp_array1.clear();

				if (label_str.compare("left_detections") == 0)
				{
					ExtractNumsFromString<float>(tmp_str, tmp_array, 1);
					ft[0] = tmp_array[0];
					ft[1] = tmp_array[1];
					scale = tmp_array[2];
					weight= tmp_array[3];

					left_detections.push_back(ft);
					left_scales.push_back(scale);
					left_weights.push_back(weight);
				}
				else if (label_str.compare("right_detections") == 0)
				{
					ExtractNumsFromString<float>(tmp_str, tmp_array, 1);
					ft[0] = tmp_array[0];
					ft[1] = tmp_array[1];
					scale = tmp_array[2];
					weight= tmp_array[3];

					right_detections.push_back(ft);
					right_scales.push_back(scale);
					right_weights.push_back(weight);
				}
				else if (label_str.compare("detection_links") == 0)
				{
					std::pair<unsigned int, unsigned int> tmp_pair;
					ExtractNumsFromString<unsigned int>(tmp_str, tmp_array1, 2);
					tmp_pair.first = tmp_array1[0];
					tmp_pair.second = tmp_array1[1];

					detection_links.push_back(tmp_pair);
				}
				else if (label_str.compare("final_detections") == 0)
				{
					ExtractNumsFromString<float>(tmp_str, tmp_array, 1);
					ft[0] = tmp_array[0];
					ft[1] = tmp_array[1];
					final_detections.push_back(ft);
				}
				else if (label_str.compare("edited_detections") == 0)
				{
					ExtractNumsFromString<float>(tmp_str, tmp_array, 1);
					ft[0] = tmp_array[0];
					ft[1] = tmp_array[1];
					edited_detections.push_back(ft);
				}
			}
		}
		myfile.close();
		if (0 == edited_detections.size())
			edited_detections = final_detections;
	}
}

bool radBackup::IsRecordExisted(string & file_name, string & searched_str)
{
	string line;
	ifstream myfile(file_name.c_str());

	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			if (line.compare(searched_str) == 0)
			{
				myfile.close();
				return true;
			}
		}
		myfile.close();
		return false;
	}
	else 
		return false;
}

bool radBackup::IsDiretoryExisted(pair<string, string> & dir_names)
{
	QDir cur_dir(BackupDir.c_str());
	cur_dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = cur_dir.entryList();

	for (int i=0; i<dirList.size(); i++)
	{
		if (dirList[i].toStdString().find(dir_names.second) != string::npos)
		{
			QString newPath = QString("%1/%2").arg(cur_dir.absolutePath()).arg(dirList.at(i));
			string filename = newPath.toStdString()+"/"+dir_names.second+".txt";
			if (IsRecordExisted(filename, dir_names.first))
				return true;
		}
	}

	return false;
}

int radBackup::GetDirectoryIndex(pair<string, string> & dir_names)
{
	QDir cur_dir(BackupDir.c_str());
	cur_dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = cur_dir.entryList();

	for (int i=0; i<dirList.size(); i++)
	{
		if (dirList[i].toStdString().find(dir_names.second) != string::npos)
		{
			QString newPath = QString("%1/%2").arg(cur_dir.absolutePath()).arg(dirList.at(i));
			string filename = newPath.toStdString()+"/"+dir_names.second+".txt";
			if (IsRecordExisted(filename, dir_names.first))
			{
				if (dirList[i].toStdString().length() == dir_names.second.length())
					return 0;
				else
				{
					string tmp_str = dirList[i].toStdString();
					tmp_str.assign(tmp_str.begin()+dir_names.second.length(), tmp_str.end());
					return atoi(tmp_str.c_str());
				}
			}
		}
	}

	return -1;
}

int radBackup::GetMaximumDirectoryIndex(pair<string, string> & dir_names)
{
	QDir cur_dir(BackupDir.c_str());
	cur_dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = cur_dir.entryList();
	int result_id = -1;

	for (int i=0; i<dirList.size(); i++)
	{
		if (dirList[i].toStdString().find(dir_names.second) != string::npos)
		{
			if (dirList[i].toStdString().length() != dir_names.second.length())
			{
				string tmp_str = dirList[i].toStdString();
				tmp_str.assign(tmp_str.begin()+dir_names.second.length(), tmp_str.end());
				if (result_id < atoi(tmp_str.c_str()))
					result_id = atoi(tmp_str.c_str());
			}
			else
			{
				if (result_id < 0)
					result_id = 0;
			}
		}
	}

	return result_id;
}

void radBackup::WriteBackupResults(SplitImageInformation & split_info,  
								   ConeDetectionParameters & detection_paras, int features_only)
{
	string filename, filename1;
	string file_dir;

	if (IsDiretoryExisted(split_info.split_files))
	{
		int dir_id = GetDirectoryIndex(split_info.split_files);
		if (dir_id <= 0)
		{
			file_dir = BackupDir + "/" + split_info.split_files.second;
		}
		else
			file_dir = BackupDir + "/" + split_info.split_files.second+QString::number(dir_id).toStdString();

		QDir infor_dir(file_dir.c_str());
		RemoveDir(infor_dir.absolutePath());
		infor_dir.mkpath(".");
	}
	else
	{
		int dir_id = GetMaximumDirectoryIndex(split_info.split_files);
		if (dir_id < 0)
		{
			file_dir = BackupDir + "/" + split_info.split_files.second;
		}
		else
			file_dir = BackupDir + "/" + split_info.split_files.second+QString::number(dir_id+1).toStdString();

		QDir infor_dir(file_dir.c_str());
		infor_dir.mkpath(".");
	}

	filename1.assign(file_dir + "/" + split_info.split_files.second + "_split_features.csv");
	WriteSplitFeatures(filename1.c_str(), split_info.split_left_detections, split_info.split_right_detections,
		split_info.split_left_detection_scales, split_info.split_right_detection_scales,
		split_info.split_left_detection_weights, split_info.split_right_detection_weights,
		split_info.split_detection_links, split_info.split_final_detections, split_info.split_edited_detections);
	if (features_only) return;

	filename.assign(file_dir + "/" + split_info.split_files.second + ".txt");

	ofstream myfile;
	myfile.open(filename.c_str());

	//recording time
	time_t rawtime;
	time(&rawtime);
#ifdef _MSC_VER
	char ctime_buf[32];
	ctime_s(ctime_buf, 32, &rawtime);
#else
	char *ctime_buf = ctime(&rawtime);
#endif

	myfile << "Created time: " << ctime_buf << std::endl;
	if (split_info.split_edited_detections.size() > 0) {
		myfile << "##VotingRadius: " << detection_paras.VotingRadius << std::endl;
		myfile << "##GradientMagnitudeThreshold: " << detection_paras.GradientMagnitudeThreshold << std::endl;
		myfile << "##Scale: " << detection_paras.Scale << std::endl;
		myfile << "##DimConeDetectionFlag: " << int(detection_paras.DimConeDetectionFlag) << std::endl;
		myfile << "##LOGResponse: " << detection_paras.LOGResponse << std::endl;
	}

	myfile << split_info.split_files.first << std::endl;
	myfile << split_info.split_files.second << std::endl;

	myfile << filename1 << std::endl;

	myfile.close();
}
