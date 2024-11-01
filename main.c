#include <16F877A.h>
#include <stdlib.h>
#FUSES XT, NOWDT, NOPROTECT, NOBROWNOUT, NOLVP, NOPUT, NODEBUG, NOCPD
#use delay(crystal=20000000)

#use rs232 (baud=9600,xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=1)
#DEFINE IN1 PIN_C3
#DEFINE IN2 PIN_C4
int counter = 0;
char strInput[16];
unsigned long inputString;
unsigned long int revAngle = 0.0f;
unsigned long int prevAngle = 0.0f;
signed long dx_dt = 0;
int i = 0;
int x = 0;
int y = 20;
signed long int result_1;
signed long int wref;
signed long int wact;
signed long long int error;
signed long int controlout;
signed long long int total_error;
float kp = 0.1;
float ki = 0.001;
float dt;

#int_ext
void external_interrupt()
{
    revAngle++;
}

#int_timer0
void tmr_int()
{
    set_timer0(60);
    x++;
    if (x >= y)
    {
        dx_dt = (revAngle - prevAngle) * (15.79 / y);
        prevAngle = revAngle;
        total_error = ((error * dt) + (total_error));
        x = 0;
    }
}

void main()
{
    setup_psp(PSP_DISABLED);
    setup_timer_1(T1_DISABLED);
    setup_timer_2(T1_DISABLED, 0, 1);
    setup_CCP1(CCP_OFF);
    setup_CCP2(CCP_OFF);

    port_b_pullups(TRUE);
    enable_interrupts(GLOBAL);
    clear_interrupt(int_ext);
    setup_timer_0(RTCC_INTERNAL | RTCC_DIV_256);
    set_timer0(60);
    enable_interrupts(int_timer0);
    enable_interrupts(int_ext);
    setup_adc_ports(AN0_AN1_AN3);
    setup_adc(ADC_CLOCK_DIV_32);

    setup_ccp1(CCP_PWM);
    setup_timer_2(T2_DIV_BY_16, 255, 1);
    set_pwm2_duty(0);
    output_low(IN1);
    output_high(IN2);
    dt = 0.01 * y;

    while (1)
    {
        if (kbhit())
        {
            char i = getc();

            if (i == 'p' || i == 'i' || i == 's')
            {
                inputString = atol(strInput);
                if (i == 'p')
                {
                    kp = inputString * 0.1;
                }
                else if (i == 'i')
                {
                    ki = inputString * 0.001;
                }
                else if (i == 's')
                {
                    y = inputString;
                    dt = 0.01 * y;
                }
                memset(strInput, 0, sizeof(strInput));
                counter = 0;
            }
            else
            {
                strInput[counter] = i;
                counter++;
                if (counter >= sizeof(strInput))
                {
                    counter = sizeof(strInput) - 1;
                }
            }
        }

        set_adc_channel(0);
        delay_us(10);
        result_1 = read_adc();
        wref = (result_1) * (9.78);
        wact = dx_dt;
        error = (wref - wact);
        controlout = (error * kp) + (total_error * ki);

        if (controlout > 1023) {
            controlout = 1023;
        }
        else if (controlout < 0) {
            controlout = 0;
        }
        set_pwm1_duty(controlout);
        delay_ms(100);
        printf("\n motor pwm :%ld", controlout);
        printf("\n actual velocity :%ld", wact);
        printf("\n reference velocity :%ld", wref);
        printf("\n kp:%f", kp);
        printf("\n ki:%.3f", ki);
        printf("\n y=%d", y);
        printf("\n sampling time:%f", dt);
        printf("\n wait for 2sec");
        delay_ms(2000);
    }
}

