// power.c

/*
    TODO:
    1. 전체적으로 함수 주석 및 지역 변수, 전역 변수, 구조체 변수 등의 변수 주석 추가 필요
    2. 함수 주석은 함수 구현부의 함수 이름 위에 함수의 동작을 설명해야 함
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF 1000

typedef struct
{
    double energy_now_wh;
    double energy_now_percent;
    double power_now_w;
    double remaining_hours;
} POWER_CAL;

typedef struct
{
    int energy_now;
    int energy_full;
    int power_now;
    char model_name[MAX_BUF];
    char status[MAX_BUF];
    POWER_CAL *power_cal;
} POWER;

void power(char* []);
void showPowerConsumption(void);
void calculatePowerConsumption(POWER*);
void showPowerReport(void);
void reportPower(POWER*);
void comparePowerConsumption(void); // 미완
void powerHelp(void);
FILE *openFile(const char*);

/* /sys/class/power_supply/BAT1/...: 컴퓨터의 배터리 정보를 기록하는 파일 */

// 문제점 및 해결과제:
// 1. 모든 리눅스 시스템에 이 파일이 존재하는 지는 확실치 않음.
// 2. 전기 콘센트를 사용하는 데스크톱에선 BAT 파일이 없음. 즉 이 방법으로 전력 사용량 측정이 불가능함.
const char *energy_now_path = "/sys/class/power_supply/BAT1/energy_now";
const char *energy_full_path = "/sys/class/power_supply/BAT1/energy_full";
const char *power_now_path = "/sys/class/power_supply/BAT1/power_now";
const char *model_name_path = "/sys/class/power_supply/BAT1/model_name";
const char *status_path = "/sys/class/power_supply/BAT1/status";
const char *power_report_path = ".eco-shell_power_report";

void power(char *power_args[]){
    if (power_args[1] == NULL)
    {
        powerHelp();
        return;
    }

    if (strcmp(power_args[1], "-i") == 0)
    {
        showPowerConsumption();
    }
    else if (strcmp(power_args[1], "-r") == 0)
    {
        showPowerReport();
    }
    else if (strcmp(power_args[1], "-c") == 0)
    {
        comparePowerConsumption();
    }
    else if (strcmp(power_args[1], "-h") == 0)
    {
        powerHelp();
    }
    else
    {
        fprintf(stderr, "%s: 이런 옵션은 없습니다.\n'power -h'를 입력하여 사용법을 확인하세요.\n", power_args[1]);
    }
}

void showPowerConsumption(void)
{
    POWER_CAL power_cal = {0};
    POWER power = {0, 0, 0, "", "", &power_cal};
    static int is_first = 1; // 0: 처음이 아님, 1: 처음임

    calculatePowerConsumption(&power);

    // 쉘이 처음 실행될 때 전력 소모량을 계산하여 기록함. 하지만 결과를 출력하지는 않음
    // 따라서 power 명령이 처음 실행 되었을 때를 제외하고 결과를 출력해야 함.
    if (is_first)
    {
        is_first = 0;

        return;
    }
    else
    {
        printf("현재 전력 소비량: %.2fW\n", power.power_cal->power_now_w);
        printf(
            "현재 배터리 잔량: %.2fWH(%.2f%%)\n", power.power_cal->energy_now_wh, power.power_cal->energy_now_percent);

        if (power.power_cal->remaining_hours > 0)
        {
            printf("남은 배터리 지속 시간: %.2fH\n", power.power_cal->remaining_hours);
        }
        else
        {
            printf("남은 배터리 지속 시간: 계산 불가\n");
        }

        printf("배터리 상태: %s\n", power.status);
        printf("배터리 모델명: %s\n", power.model_name);
    }
}

void calculatePowerConsumption(POWER *power)
{
    FILE *fp_energy_now = openFile(energy_now_path);
    FILE *fp_energy_full = openFile(energy_full_path);
    FILE *fp_power_now = openFile(power_now_path);
    FILE *fp_model_name = openFile(model_name_path);
    FILE *fp_status = openFile(status_path);

    char buf[MAX_BUF];

    if (fgets(buf, sizeof(buf), fp_energy_now) != NULL)
        power->energy_now = atoi(buf);

    if (fgets(buf, sizeof(buf), fp_energy_full) != NULL)
        power->energy_full = atoi(buf);

    if (fgets(buf, sizeof(buf), fp_power_now) != NULL)
        power->power_now = atoi(buf);

    if (fgets(buf, sizeof(buf), fp_model_name) != NULL)
    {
        strncpy(power->model_name, buf, MAX_BUF - 1);
        power->model_name[MAX_BUF - 1] = '\0';
    }

    if (fgets(buf, sizeof(buf), fp_status) != NULL)
    {
        buf[strcspn(buf, "\n")] = '\0';
        if (strcmp(buf, "Discharging") == 0)
        {
            strncpy(power->status, "충전 중이 아님", MAX_BUF - 1);
        }
        else if (strcmp(buf, "Charging") == 0)
        {
            strncpy(power->status, "충전 중", MAX_BUF - 1);
        }
        else
        {
            strncpy(power->status, "충전 상태 확인 불가", MAX_BUF - 1);
        }
    }

    power->power_cal->energy_now_wh = (double)power->energy_now / 1000000;
    power->power_cal->energy_now_percent = ((double)power->energy_now / power->energy_full) * 100;
    power->power_cal->power_now_w = (double)power->power_now / 1000000;
    power->power_cal->remaining_hours =
        (power->power_cal->power_now_w > 0) ? power->power_cal->energy_now_wh / power->power_cal->power_now_w : -1;

    fclose(fp_energy_now);
    fclose(fp_energy_full);
    fclose(fp_power_now);
    fclose(fp_model_name);
    fclose(fp_status);

    reportPower(power);
}

void reportPower(POWER *power)
{
    FILE *fp_report_power = openFile(power_report_path);
    static int idx = 1;

    fprintf(fp_report_power, "%d %f %f\n", idx++, power->power_cal->power_now_w, power->power_cal->energy_now_wh);
    fclose(fp_report_power);
}

void showPowerReport(void)
{
    FILE *fp_report_power = openFile(power_report_path);
    char buf[MAX_BUF];

    printf("%-10s %-10s %-10s\n", "번호", "전력 소모량", "배터리 잔량");
    while ((fgets(buf, sizeof(buf), fp_report_power)) != NULL)
    {
        printf("%s", buf);
    }

    fclose(fp_report_power);
}

/* comparePowerConsumption(): 전력 사용량 비교를 위한 함수 */

// TODO:
// 1. 쉘 초기 실행시 전력 상황 기록.(완료)
// 2. 사용자가 전력 사용량 비교 명령 요청시 파일 값을 읽어 비교.
//  -> power -c 명령
// 3. 쉘 실행 시간 동안의 전력 소모 속도, 소모된 양, 배터리 사용된 시간 등 출력
// 4. 전력 소모량을 바탕으로 쉘 실행 시간 동안의 탄소 배출량 추산.
void comparePowerConsumption(void) 
{
    
}

void powerHelp(void)
{
    printf("power: 전력 사용량 확인 명령어.\n");
    printf("사용법: fclean <옵션>\n");
    printf("  -i: 배터리 잔량, 배터리 소모량, 남은 배터리 사용 시간, 배터리 정보 출력\n");
    printf("  -r: power 명령어 사용 기록 확인.\n");
    printf("  -c: 최근 power 명령어 사용 이후 전력 소모량 비교\n");
    printf("  -h: 도움말\n");
}

FILE *openFile(const char *path)
{
    FILE *file;

    if (strcmp(path, power_report_path) == 0)
    {
        file = fopen(path, "a+");
    }
    else
    {
        file = fopen(path, "r");
    }

    if (file == NULL)
    {
        perror(path);
        exit(EXIT_FAILURE);
    }

    return file;
}