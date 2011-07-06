#include <stdio.h>
#include <sys/time.h>
#define MAX_GT_LEN 24

typedef unsigned int UINT;
typedef unsigned short USHORT;

typedef struct {
	time_t	time;
    UINT    uiSSN;
    char    szGT[MAX_GT_LEN];

    USHORT  usResult; 
    short   sSysNo;   
    
}st_SG_Msg, *pst_SG_Msg ;

typedef struct _st_SG{
    char    szGT[MAX_GT_LEN]; /* KEY */

    int     dAlarm;           
    UINT    uiTrial;
    UINT    uiSuccess; 
    
} st_SG, *pst_SG;

#define MAX_SG_RECORD 100    /* AO￠￥e ¡icAIAi￠￥A 100¡Æⓒø AI¡ioAI ￥iE¨uo￥i￥i AOA¨o */
typedef struct _st_SG_List{
    USHORT  usCount;      /* Type¨￢¡Æ Count */
    USHORT  usType;       /* Type : HLR, SCP, VLR, SMC, EIR */
    st_SG   stSG[MAX_SG_RECORD];
}st_SG_List,*pst_SG_List;
    
#define MAX_SG_TYPE 10  
typedef struct _Watch_SG_Data{
    int     dTime;
    UINT    uiMSCID;

    st_SG_List  stSGList[MAX_SG_TYPE];
}st_Watch_SG_Data, *pst_Watch_SG_Data;


/* DB￠?￠®¨u¡ⓒ A¨￠¨ui￠?A¡¾a A¡×CN ¡¾￠￢A￠OA¨u */
#define MAX_SSN_GT_RECORD (MAX_SG_RECORD*MAX_SG_TYPE)    /* 100 * 10 */
typedef struct _ST_SSN_GT{

 UINT uiSSN;
 char szGT[MAX_GT_LEN];

}st_SSN_GT,*pst_SSN_GT;


#define DEF_MYLEN 125
int main()
{
	int a=1;
	struct _st{
		int a;
		char c;
	} stMY[3], *pstMY;

	printf("a=%d\n",a);
	stMY[0].a = a;
	stMY[1].a = a+1;
	printf("ss[0]=%d\n",stMY[0].a);
	printf("ss[1]=%d\n",stMY[1].a);
	stMY[1].a = a+3;

	pstMY = &stMY[1];
	printf("pss=%d\n",pstMY->a);
	myfunc(DEF_MYLEN);
	return 0;
}

int myfunc(int dLen){
	struct _st{
		int a;
		char sz[dLen];
	}STMY;
	printf("dLen=%d\n",dLen);
	printf("STMYSIZE=%d\n",sizeof(STMY));
	printf("STMYLEN=%d\n",strlen(STMY.sz));
	return 0;
}
