#include <winsock.h>
#include <Iphlpapi.h>
#include <iostream> 
#include <fstream>
#include <time.h>
#include <comutil.h>
#include <windows.h>
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
//----------------------------------------------------------------------------
inline VOID ADD_IP_FORWARD_ENTRY(const char* pIP, const char* pMASK,PMIB_IPFORWARDROW pipfr);
//----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR szCmdLine,int iCmdShow)
{
	MIB_IPFORWARDROW ipfr={0}; 
	ipfr.dwForwardAge=0; 
	ipfr.dwForwardMetric2=0xFFFFFFFF; 
	ipfr.dwForwardMetric3=0xFFFFFFFF; 
	ipfr.dwForwardMetric4=0xFFFFFFFF; 
	ipfr.dwForwardMetric5=0xFFFFFFFF; 
	ipfr.dwForwardNextHop=inet_addr("192.168.88.1"); 
	ipfr.dwForwardNextHopAS=0; 
	ipfr.dwForwardPolicy=0; 
	ipfr.dwForwardProto=3; 
	ipfr.dwForwardType=4;
	//-------------------------------------------取得dwForwardIfIndex
	PIP_ADAPTER_INFO pinfo=NULL; 
	unsigned long len=0; 
	unsigned long nError; 
	nError=GetAdaptersInfo(pinfo,&len); 
	pinfo=(PIP_ADAPTER_INFO)malloc(len); 
	nError=GetAdaptersInfo(pinfo,&len); 
	if(nError==0) {
		while(pinfo!=NULL && (inet_addr(pinfo->IpAddressList.IpAddress.String) & inet_addr("255.255.255.0"))
			!= inet_addr("192.168.88.0")) {               
				//      MessageBox(NULL,_com_util::ConvertStringToBSTR(pinfo->Type),
				//              _com_util::ConvertStringToBSTR(pinfo->IpAddressList.IpAddress.String),0);
				pinfo=pinfo->Next;
		}
		if(pinfo==NULL) {
			MessageBox(NULL,TEXT("未找到合适的网卡接口"),TEXT("WRONG"),16);
			return 0;
		} else {
			ipfr.dwForwardIfIndex = pinfo->Index;
		}
	} else { 
		if(nError==ERROR_NO_DATA) 
			MessageBox(NULL,TEXT("请检查您的计算机是否安装了网卡"),TEXT("WRONG"),16); 
		if(nError==ERROR_NOT_SUPPORTED)
			MessageBox(NULL,TEXT("操作系统不匹配"),TEXT("WRONG"),16);
	}
	free(pinfo); 
	//-------------------------------------------获得dwForwardMetric1的值
	PMIB_IPFORWARDTABLE pIpForwardTable;
	pIpForwardTable =
		(MIB_IPFORWARDTABLE *) malloc(sizeof (MIB_IPFORWARDTABLE));
	if (pIpForwardTable == NULL) {
		printf("Error allocating memory\n");
		return 1;
	}
	DWORD dwSize = 0;
	if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) ==
		ERROR_INSUFFICIENT_BUFFER) {
			free(pIpForwardTable);
			pIpForwardTable = (MIB_IPFORWARDTABLE *) malloc(dwSize);
			if (pIpForwardTable == NULL) {
				printf("Error allocating memory\n");
				return 1;
			}
	}
	if ((GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) {
		for(DWORD i=0;i<pIpForwardTable->dwNumEntries;++i){
			if(pIpForwardTable->table[i].dwForwardNextHop==inet_addr("192.168.88.1")){
				ipfr.dwForwardMetric1 = pIpForwardTable->table[i].dwForwardMetric1+1;
				break;
			}
		}
	}
	free(pIpForwardTable);

	FILE *fp = fopen("C:\\routes.txt","rb");
	if	(fp==NULL) {
		MessageBox(NULL,TEXT("预设路由表打开失败"),TEXT("ERROR"),0);
		return 0;
	}
	char currChar = 0;
	char ip[20];
	char mask[20];
	char currPos=0;
	int state = 0;//0,读ip；1，读mask
	while (fread(&currChar,1,1,fp)!=0) {
		if (isspace(currChar)) {
			if (state==0) {
				ip[currPos] = '\0';
				currPos = 0;
				while (fread(&currChar,1,1,fp)!=0) {
					if (isspace(currChar)) {
						continue;
					}
					fseek(fp,-1,SEEK_CUR);
					break;
				}
				state = 1;
			} else if (state==1) {
				mask[currPos] = '\0';
				currPos = 0;
				ADD_IP_FORWARD_ENTRY(ip,mask,&ipfr);
				while (fread(&currChar,1,1,fp)!=0) {
					if (isspace(currChar)) {
						continue;
					}
					fseek(fp,-1,SEEK_CUR);
					break;
				}
				state = 0;
			}
			continue;
		}
		if (state==0) {
			ip[currPos++] = currChar;
		} else if (state==1) {
			mask[currPos++] = currChar;
		}

	}

	fclose(fp);
	MessageBox(NULL,TEXT("添加路由结束"),TEXT("Finished"),0);
	return 0;
}//---------------------------------------------------------------------------
inline VOID ADD_IP_FORWARD_ENTRY(const char* pIP, const char* 
	pMASK,PMIB_IPFORWARDROW pipfr) 
{
	pipfr->dwForwardDest=inet_addr(pIP); 
	pipfr->dwForwardMask=inet_addr(pMASK); 
	DWORD dwRet=CreateIpForwardEntry(pipfr);
	if(dwRet!=0)
		MessageBox(NULL,_com_util::ConvertStringToBSTR(pIP),TEXT("添加路由项目失败"),16);

}//===========================================================================
