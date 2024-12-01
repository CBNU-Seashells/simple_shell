// main.c

#include <stdio.h>

void runShell(void);
void calculatePowerConsumption(void);

int main(void)
{
    printf("Welcome to Eco-Shell!\n");

    // .eco-shell_power_report 파일 초기화를 위한 삭제
    remove(".eco-shell_power_report");

    // 전력 소모량 초기 기록을 위한 power 커맨드 자동 실행
    calculatePowerConsumption();
    runShell();

    return 0;
}