// command.c

/*
    TODO
    특수 명령어에는 환경 친화적 기능 명령어 함수가 들어가야 함
    프로세스 정리, 파일 정리, 에코모드 구현하기
*/

#include <stdio.h>
#include <string.h>

#define ARGS_BUFFER 50

void showPowerConsumption(void);
void showPowerReport(void);

void cleanupFiles(char *[]);
void cleanupProcess(void);
void ecoMode(void);

// 일반적인 쉘 명령어를 처리할 때 사용된다.
int runProcess(char *[]);

// 명령행에서 입력받은 명령어를 처리한다.
int executeCommand(char *input)
{
    /*
        명령행에서 입력받은 하나의 전체 문자열을 strtok을 통해 공백(" ")을 기준으로 잘라서
        토큰화된 문자열을 저장하는 포인터 배열
    */
    char *args[ARGS_BUFFER];

    /*
        명령행에서 입력받은 문자열을 공백(" ")을 기준으로 자른
        토큰화된 문자열 하나(명령행 인자)를 임시로 저장하는 버퍼
    */
    char *token;

    int i = 0;

    // 명령어 토큰화
    token = strtok(input, " ");

    // 명령행에서 입력받은 문자열의 토큰화 처리 과정
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }

    // 마지막 명령행 인자 뒤에 NULL 종료 값을 할당함으로써 배열의 끝을 나타냄
    args[i] = NULL;

    // 환경 친화 명령어 실행
    if (strcmp(args[0], "power") == 0)
    {
        if (args[1] == NULL)
        {
            // 전력 소모량 출력 명령 - power
            showPowerConsumption();
        }
        else if (strcmp(args[1], "-r") == 0)
        {
            // power 명령어 사용 로그(전력 소모량, 배터리 잔량 출력) - power -r
            showPowerReport();
        }
        else
        {
            printf("Invalid Option: %s\n", args[1]);
            return 1;
        }

        return 0;
    }
    else if (strcmp(args[0], "fclean") == 0)
    {
        // 불필요한 파일 정리 명령 - fclean
        cleanupFiles(args);

        return 0;
    }
    else if (strcmp(args[0], "pclean") == 0)
    {
        // 불필요한 프로세스 종료 명령 - pclean
        cleanupProcess();

        return 0;
    }
    else if (strcmp(args[0], "ecomode"))
    {
        // 에코모드 - ecomode
        // 추후 옵션 인자 처리를 위해 주석으로 args를 넣었음
        ecoMode(/*args*/);
    }

    // 일반 쉘 명령어 실행
    return runProcess(args);
}