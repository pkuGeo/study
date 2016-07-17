/*
 * main.cpp
 *
 *  Created on: 2013年12月31日
 *      Author: geo
 */




#include <iostream>
#include <cstdlib>
#include <memory>
#include <mongo/client/dbclient.h>
#include <json/json.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

std::string convertC2ByteVal(const std::string& originStr)
{
    std::string converted;
    char num[7] = {'\0'};
    const unsigned char *pchar = (const unsigned char*)originStr.c_str();
    while (*pchar!='\0') {
        unsigned char thischar = *pchar;
        if (thischar<128) {
            //ascii
            converted.append(1,(char)thischar);
            ++pchar;
            continue;
        } else if ((thischar&0xE0)==0xC0) {
            //110xxxxx 10xxxxxx
            unsigned char num[2];
            num[0] = thischar & 0x1F;
            num[1] = ((*++pchar) & 0x3F) | (num[0]<<6) ;
            num[0] >>= 2;
            char str[7];
            snprintf(str, 7, "\\u%02x%02x",num[0],num[1]);
            converted.append(str);
            ++pchar;                                                                            
        } else if ((thischar&0xF0)==0xE0) {
            //chinese character
            thischar <<= 4;
            unsigned char char2 = *++pchar;
            unsigned char char3 = *++pchar;
            char2 &= 0x3F;
            char3 &= 0x3F;
            thischar |= (char2>>2);
            char3 |= (char2<<6);
            snprintf(num,7,"\\u%02x%02x",thischar,char3);
            converted.append(num);
            ++pchar;
        } else {
            std::cerr<<"!!!error character:"<<thischar<<std::endl;
            std::cerr<<pchar<<std::endl;
            return std::string();
        }
    }
    return converted;

}

void setJsonOrder(Json::Value &root)
{
	bool suc = false;

	suc = root.setMemberOrder("building",1);
	if (!suc) {
		std::cerr<<"ERROR OCCURED"<<std::endl;
	}

	suc = root.setMemberOrder("Floors",2);
	if (!suc) {
		std::cerr<<"ERROR OCCURED"<<std::endl;
	}

	Json::Value &buildingObj = root["building"];
	suc = buildingObj.setMemberOrder("_xLon",1);
	if (!suc) {
		std::cerr<<"ERROR OCCURED"<<std::endl;
	}

	suc = buildingObj.setMemberOrder("_yLat",2);
	if (!suc) {
		std::cerr<<"ERROR OCCURED"<<std::endl;
	}

    suc = buildingObj.setMemberOrder("Outline",3);
    if (!suc) {
        std::cerr<<"ERROR OCCURED"<<std::endl;
    }

	Json::Value &floorsObj = root["Floors"];
	for (unsigned int i = 0; i < floorsObj.size(); ++i) {
		Json::Value &floorObj = floorsObj[i];
		std::vector<std::string> members = floorObj.getMemberNames();
		for (std::vector<std::string>::iterator iter = members.begin();
				iter != members.end();
				++iter) {
			if (*iter=="_id") {
				suc = floorObj.setMemberOrder(*iter,1);
			} else if (*iter=="Outline") {
				suc = floorObj.setMemberOrder(*iter,3);
			} else if (*iter=="PubPoint") {
				suc = floorObj.setMemberOrder(*iter,4);
			} else if (*iter=="FuncAreas") {
				suc = floorObj.setMemberOrder(*iter,5);
			} else {
				suc = floorObj.setMemberOrder(*iter,2);
			}
			if (!suc) {
				std::cerr<<"ERROR OCCURED"<<std::endl;
			}
		}
	}
}

int main(int argc, char** argv)
{

//    Json::Value top;
//    top["code"]="1";
//    top.setMemberOrder("code",10);
//
//    top["message"]="Successful.";
//    top.setMemberOrder("message",9);
//
//    top["result"]="true";
//    top.setMemberOrder("result",8);
//
//    top["timestamp"]="1388471797.82";
//    top.setMemberOrder("timestamp",7);
//
//    top["version"]="2.0-2.0.3346.1735";
//    top.setMemberOrder("version",6);
//
//    std::string topstr = Json::StyledWriter().write(top);
//    std::cout<<topstr<<std::endl;
//    return 0;
	mongo::DBClientConnection conn;
	try {
		conn.connect("10.5.100.41");
        std::auto_ptr<mongo::DBClientCursor> indexCursor = conn.query("indoor.buildings_Index", mongo::BSONObj());
        if (!indexCursor->more()) {
            std::cerr<<"Error: buildings_Index is empty!"<<std::endl;
            return -1;
        }
        Json::Value idxRoot;
        while (indexCursor->more()) {
            Json::Reader reader;
            std::string buildIdxJson = indexCursor->next().jsonString();
            Json::Value buildIdxVal;
            if (!reader.parse(buildIdxJson,buildIdxVal)) {
                std::cerr<<"Parse Error!!!"<<std::endl;
                std::cerr<<buildIdxJson<<std::endl;
            }
            idxRoot.append(buildIdxVal);
        }
        std::string idxJson = Json::FastWriter().write(idxRoot);
        FILE *idxfp = fopen("idx.json","w");
        fwrite(idxJson.c_str(),1,idxJson.size(),idxfp);
        fflush(idxfp);
        fclose(idxfp);
        std::string idxUJson = convertC2ByteVal(idxJson);
        idxfp = fopen("idx.json.val","w");
        fwrite(idxUJson.c_str(),1,idxUJson.size(),idxfp);
        fflush(idxfp);
        fclose(idxfp);

		std::auto_ptr<mongo::DBClientCursor> cursor = conn.query("indoor.buildings",mongo::BSONObj());
		while (cursor->more()) {
			std::string json = cursor->next().jsonString();
            Json::Value root;
			Json::Reader reader;
			bool parseSuc = reader.parse(json, root);
			if (!parseSuc) {
				std::cerr<<"Parse Error!!!"<<std::endl;
			    std::cerr<<json<<std::endl;
                return -1;
			}
			setJsonOrder(root);
			std::string bldgID = root["_id"].asString();
			mkdir(bldgID.c_str(),0755);
			Json::Value floors = root["Floors"];
			for (unsigned int i = 0; i < floors.size(); ++ i) {
				Json::Value newroot = root;
				Json::Value newfloors;
				newfloors.append(floors[i]);
				for (unsigned int j = 0; j < floors.size();++j) {
					if(i == j) {
						continue;
					}
					newfloors.append(floors[j]);
				}
				newroot["Floors"]=newfloors;
				//setJsonOrder(newroot);
				Json::FastWriter jsonWriter;
				std::string strJson = jsonWriter.write(newroot);
				int floor = floors[i]["_id"].asInt();
				char czFloor[20];
				snprintf(czFloor,20,"%d",floor);
				std::string jsonFilePath = bldgID+"/"+czFloor+".json";
				FILE *fp = fopen(jsonFilePath.c_str(),"w");
				fwrite(strJson.c_str(),1,strJson.size(),fp);
				fflush(fp);
				fclose(fp);
                std::string strUJson = convertC2ByteVal(strJson);
                jsonFilePath.append(".val");
                fp = fopen(jsonFilePath.c_str(),"w");
                fwrite(strUJson.c_str(),1,strUJson.size(),fp);
                fflush(fp);
                fclose(fp);
			}
			//setJsonOrder(root);
			Json::FastWriter jsonWriter;
			std::string strJson = jsonWriter.write(root);
			std::string jsonFilePath = bldgID+"/"+"default.json";
			FILE *fp = fopen(jsonFilePath.c_str(),"w");
			fwrite(strJson.c_str(),1,strJson.size(),fp);
			fflush(fp);
			fclose(fp);
            std::string strUJson = convertC2ByteVal(strJson);
            jsonFilePath.append(".val");
            fp = fopen(jsonFilePath.c_str(),"w");
            fwrite(strUJson.c_str(),1,strUJson.size(),fp);
            fflush(fp);
            fclose(fp);
		}
	} catch (const mongo::DBException &e ) {
		std::cerr<<"Caught: "<<e.what()<<std::endl;
	}
    

}
