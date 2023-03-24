#include <stdio.h>
#include <windows.h>
#include <string.h>

#if 1   //����DEBUG��ӡ
#define LOGD(...) printf(__VA_ARGS__)
#else   //�ر�DEBUG��ӡ
#define LOGD(...)
#endif

#if 1   //����ERROR��ӡ
#define LOGE(...) printf(__VA_ARGS__)
#else   //�ر�ERROR��ӡ
#define LOGE(...)
#endif

//��������С
#define BUF_SIZE    2048
#define EXIT_STR    "exit"
#define I_EXIT      "I exit.\r\n"
#define I_RECEIVE   "I receive.\r\n"

//�򿪴���
HANDLE OpenSerial(const char* com, //�������ƣ���COM1��COM2
    int baud,       //�����ʣ�����ȡֵ��CBR_9600��CBR_19200��CBR_38400��CBR_115200��CBR_230400��CBR_460800
    int byteSize,   //��λ��С����ȡֵ7��8��
    int parity,     //У�鷽ʽ����ȡֵNOPARITY��ODDPARITY��EVENPARITY��MARKPARITY��SPACEPARITY
    int stopBits)   //ֹͣλ��ONESTOPBIT��ONE5STOPBITS��TWOSTOPBITS��
{
    DCB dcb;
    BOOL b = FALSE;
    COMMTIMEOUTS CommTimeouts;
    HANDLE comHandle = INVALID_HANDLE_VALUE;

    //�򿪴���
    comHandle = CreateFileA(com,            //��������
        GENERIC_READ | GENERIC_WRITE,      //�ɶ�����д   				 
        0,            // No Sharing                               
        NULL,         // No Security                              
        OPEN_EXISTING,// Open existing port only                     
        FILE_ATTRIBUTE_NORMAL,            // Non Overlapped I/O                           
        NULL);        // Null for Comm Devices

    if (INVALID_HANDLE_VALUE == comHandle) 
    {
        LOGE("CreateFile fail\r\n");
        return comHandle;
    }

    // ���ö�д�����С
    b = SetupComm(comHandle, BUF_SIZE, BUF_SIZE);
    if (!b) 
    {
        LOGE("SetupComm fail\r\n");
    }

    //�趨��д��ʱ
    CommTimeouts.ReadIntervalTimeout = MAXDWORD;//�������ʱ
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;//��ʱ��ϵ��
    CommTimeouts.ReadTotalTimeoutConstant = 0;//��ʱ�䳣��
    CommTimeouts.WriteTotalTimeoutMultiplier = 1;//дʱ��ϵ��
    CommTimeouts.WriteTotalTimeoutConstant = 1;//дʱ�䳣��
    b = SetCommTimeouts(comHandle, &CommTimeouts); //���ó�ʱ
    if (!b) 
    {
        LOGE("SetCommTimeouts fail\r\n");
    }

    //���ô���״̬����
    GetCommState(comHandle, &dcb);//��ȡ��ǰ
    dcb.BaudRate = baud; //������
    dcb.ByteSize = byteSize; //ÿ���ֽ���λ��
    dcb.Parity = parity; //����żУ��λ
    dcb.StopBits = stopBits; //һ��ֹͣλ
    b = SetCommState(comHandle, &dcb);//����
    if (!b) 
    {
        LOGE("SetCommState fail\r\n");
    }

    return comHandle;
}


int main(int argc, char* argv[])
{
    int Length = 0,i = 0;
    BOOL b = FALSE;
    DWORD wWLen = 0;
    char *data;
    HANDLE comHandle = INVALID_HANDLE_VALUE;//���ھ��    	

    //�򿪴���
    const char* com = "COM1";
    comHandle = OpenSerial(com, CBR_115200, 8, NOPARITY, ONESTOPBIT);
    if (INVALID_HANDLE_VALUE == comHandle) 
    {
        LOGE("OpenSerial COM1 fail!\r\n");
        return -1;
    }
    LOGD("Open COM1 Successfully!\r\n");

    FILE* fpRead = fopen("F:\\����\\��������\\�������\\��λ���\\��������\\��������.txt", "rb");//�����ļ�����

    if (fpRead == NULL)//���ļ�ʧ��
    {
        printf("Open file error!\n");
        exit(0);
    }

    else
    {
        fseek(fpRead, 0, SEEK_END); //ָ���ļ�β��ַ 
        Length = ftell(fpRead); //�����ļ����� 
        fseek(fpRead, 0, SEEK_SET); //�ָ����ļ�ָ�����׵�ַ
    }

    data = (char*)malloc(Length * sizeof(char)); //�����ļ����ȵ��ڴ�

    if (data != NULL)
    {
        fread(data, Length * sizeof(char), 1, fpRead); //һ���Զ��� 
    }

    //����
    while(i < Length)
    //for (i = 0; i < Length; i++)
    {
        b = WriteFile(comHandle, &data[i], BUF_SIZE, &wWLen, NULL);

        if (!b)
        {
            LOGE("WriteFile fail\r\n");
            continue;
        }

        if ((i + BUF_SIZE) > Length)
        {
            i = Length - i;
            Sleep(100);//��ʱ100ms
        }
        else if ((i + BUF_SIZE) == Length)
        {
           break;
        }
        else
        {
            i = i + BUF_SIZE;
            Sleep(100);//��ʱ100ms
        }

    }

    PurgeComm(comHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);//��ջ�����

    free(data);
    fclose(fpRead);

    //ѭ��������Ϣ���յ���Ϣ����Ϣ���ݴ�ӡ���ظ�I_RECEIVE, ����յ�EXIT_STR�ͻظ�EXIT_STR���˳�ѭ��
 /*   while (1) {
        wRLen = 0;*/
        //��������Ϣ
        //b = ReadFile(comHandle, buf, sizeof(buf) - 1, &wRLen, NULL);

        //if (b && 0 < wRLen)
        //{//���ɹ��������ݴ�С����0
        //    buf[wRLen] = '\0';
        //    LOGD("[RECV]%s\r\n", buf);//��ӡ�յ�������
        //    if (0 == strncmp(buf, EXIT_STR, strlen(EXIT_STR))) {
        //        //�ظ�
        //        b = WriteFile(comHandle, TEXT(I_EXIT), strlen(I_EXIT), &wWLen, NULL);
        //        if (!b) 
        //        {
        //            LOGE("WriteFile fail\r\n");
        //        }
        //        break;
        //    }
        //    //����
        //    b = WriteFile(comHandle, TEXT(I_RECEIVE), strlen(I_RECEIVE), &wWLen, NULL);
        //    if (!b) {
        //        LOGE("WriteFile fail\r\n");
        //    }
        //}

    //}

    //�رմ���
    b = CloseHandle(comHandle);
    if (!b) 
    {
        LOGE("CloseHandle fail\r\n");
    }

    LOGD("Program Exit.\r\n");
    return 0;
}