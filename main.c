#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "DSP2833x_EPwm_defines.h"
#include "DSP2833x_Device.h" // da bih mogao da koristim pozivanje funkcija kao sto su
                             // GpioDataRegs etc. Uz ovo se mora ucitati biblioteka DSP2833x_GlobalVariableDefs.c

#include "stdlib.h"
#include "math.h"
#include "stdio.h"

//#include "IQmathLib.h"


#define pi (double) 3.14159265359
#define dva_pi (double) 6.283185308
#define koren_iz_tri (double) 1.732051
#define pi_trecina (double) 1.047197
#define dva_pi_trecina (double) 2.094395
#define cetiri_pi_trecina (double) 4.188790
#define pet_pi_trecina (double) 5.2359877



// definisanje eksternih funkcija

extern void InitSysCtrl(void); // Ova funkcija disabluje WatchDog pa ce se u main rutini opet WD enablovati
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
                                // ulazi ove funkcije su :
                                // Timer koji zelimo da broji ( 0, 1 ili 2), frekvencija na kojoj radi procesor
                                // sto je 150Mhz i broj koji govori do koliko microsec zelimo da Timer broji

// funkcije koje se pozivaju u main rutini

void Gpio_select(void);
void Setup_ePWM(void);
interrupt void ePWM1A_compare_isr(void);


// definicija promenljivih
double Theta_pomocno;
double T_help;
double m=0.707;
float offset=0x7CD;
double Theta = 0, Theta_increment=0;
double frequency=0, frequency_new=0;
double fnom=50;
int ResetFLT=0, Enable=0;
double slope = 20;
float TCLK = 0.0000000067;
double fpwm=10000;
double tbprd=1875;
double sestina=0.16666666667;
double a=0;



void main(void)
{



    InitSysCtrl();  // Basic Core Init from DSP2833x_SysCtrl.c

    EALLOW;
        SysCtrlRegs.WDCR= 0x00AF;   // Re-enable the watchdog jer ga InitSysCtrl iskljuci
    EDIS;                           // 0x00AF  to NOT disable the Watchdog, Prescaler = 64

    DINT; //DISABLE na svaki interrupt, kasnije se omoguce ostali interrupti
    Gpio_select();
    Setup_ePWM();

    InitPieCtrl();      // basic setup of PIE table; from DSP2833x_PieCtrl.c
    InitPieVectTable(); // default ISR's in PIE


    EALLOW;
    PieVectTable.EPWM1_INT = &ePWM1A_compare_isr; //omoguci interrupt za ePWM1A_compare_isr interrupt rutinu
    //PieVectTable.TINT0 = &soft_start; //  -||-
    EDIS;

    //InitCpuTimers();    // basic setup CPU Timer0, 1 and 2

    //ConfigCpuTimer(&CpuTimer0,150,200000);

    PieCtrlRegs.PIEIER3.bit.INTx1 = 1; // omoguci u PIE interrup ePWM1
    PieCtrlRegs.PIEIER1.bit.INTx6 = 1; // omoguci u PIE ADC int.
    //PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // omoguci u PIE TINT0 ( Timer0 interrupt )

    IER |=4; // grupa 3 omogucen interrupt. U grupi 3 se nalazi PWM interrupt unit
    IER |=1; // grupa 1 omogucen interrupt. U grupi 1 se nalazi ADC i Timer0 interrupt unit

    // |= bitwise 'OR'

    EINT;//globalno omogucavanje prekidnih rutina
    ERTM;

    //CpuTimer0Regs.TCR.bit.TSS = 0;  // Timer krece da broji


        while(1)
        {
            EALLOW;
            SysCtrlRegs.WDKEY = 0x55;   // service WD #1
            EDIS;

        //Watchdog je brojac ciji se sadrzaj neprestano inkrementira ukoliko iz algoritma ne stigne instrukcija
        //za reset sadrzaja istog. Kada sadrzaj brojaca dostigne neku vrednosti dolazi do zaustavljanja rada MC.
        //Na taj nacin se moze detektovati greska u radu algoritma.

    //Kako se u kodu resetuje WD ? --> Tako sto se upise potrebna sekvenca
    //0x55 --> Counter enabled for reset on next 0xAA write
    //0xAA --> Counter set to zero if reset enabled ( bilo koja druga vrednost nema nikakvog efekta)


    //Watchdog ne treba "odrzavati" samo u interrupt rutini.
    //Moze da se desi da maincode krasuje, a da interrupt
    //nastavi da radi, u tom slucaju WD nece uhvatiti da je doslo do greske.
    //Zbog toga se 0x55 stavlja negde u main, a 0xAA u isr //:)  --> ovo ce da uhvati bilo koji crash

        }
}



void Gpio_select(void)
{

    // Svrha ove funkcije je dodeljivanje uloga General Purpose Input/Output - ima
    //Svi digitalni ulazi i izlazi DSC-a TMS320F28335 su grupisani u tri porta (A,B i C). Svaki od pinova moze da //izvrsava vise funkcija, ali samo jednu u datom alogoritmu. Da bi se tacno odredila koju funkciju ima koji pin //koristi se multipleksiranje. To znaci da zapravo jedan pin moze da se koristi za 4 funkcije, a na programeru je da //odluci koje ce funkcije dodeliti kom pinu.

    EALLOW;

    GpioCtrlRegs.GPAMUX1.all = 0;       // svi bitovi kao GPIO

    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // postavljanje GPIO0 kao EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1; // postavljanje GPIO1 kao EPWM1B

    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // postavljanje GPIO2 kao EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1; // postavljanje GPIO3 kao EPWM2B

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;


    // Reset_fault, ENABLE SIGNAL - ubaciti kasnije

    // zbog samog funkcionisanja invertora potrebni su nam signali reset_fault i ENABLE
    // Ukoliko bi se desilo da su oba tranzistora u nekoj od invertorskih grana ukljucena moglo bi da dodje
    // do kratkog spoja i razaranja invertora. Zbog toga se u pocetnom stanju pretpostavlja kvar i svi tranzistori
    // su iskljuceni

    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;  // postavi bitove kao outpute
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;

    EDIS;

    // EALLOW i EDIS su komande kojima se omogucava pristup zasticenim grupama u koju spadaju i varijable setovane
    // u gornjoj funkciji
}
void Setup_ePWM(void)
{
    tbprd=1875/(fpwm/10000); // prenosni odnos za odredjivanje pwm frekvencije

    //############
    //   EPWM1
    //############

    EPwm1Regs.TBCTL.bit.CLKDIV =  2;    // CLKDIV = 4
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
    EPwm1Regs.TBPRD  = tbprd; //1875 bilo,to znaci da je signal imao frekvenciju 10kHz
                         //fpwm = fsyst / ( 2*TBPRD*CLKDIV*HSPCLKDIV)
                        // fpwm = 150e6 / (2*1875*4*1) = 10e3

    EPwm1Regs.CMPA.half.CMPA  = (int) 0.5*EPwm1Regs.TBPRD; // pocetna vrednost


    EPwm1Regs.AQCTLA.all = 0x0060; // Set PWM high kada na UP count dostignes CMPA, Set PWM low kada na count down = CMPA
                               // AQCTLA == Action Qualifier

    EPwm1Regs.TBCTL.bit.SYNCOSEL  = 1; // PWM1 je izvor za sinhronizaciju sa ostala dva PWM modula i to kada je counter
                                   // dostigao nultu vrednost

    EPwm1Regs.ETSEL.bit.INTSEL = 1; // generisi PWM interrupt kada je Counter = 0;
    EPwm1Regs.ETSEL.bit.INTEN = 1;  // dozvoli da ePWM1 generise interrupt
    EPwm1Regs.ETPS.bit.INTPRD = 1; // trazi INT na svaki event (Counter = 0);
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 0;  // ucitaj kada je CMP = 0;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0; // Shadow mode enabled;


    //############
    //   EPWM2
    //############


    EPwm2Regs.TBCTL.bit.CLKDIV =  2;    // CLKDIV = 4
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
    EPwm2Regs.TBPRD  = EPwm1Regs.TBPRD;

    EPwm2Regs.CMPA.half.CMPA  = EPwm1Regs.CMPA.half.CMPA ;
    EPwm2Regs.AQCTLA.all = 0x0060;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 0;  // ucitaj kada je CMP = 0;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 0; // Shadow mode enabled;


    //############
    //   EPWM3
    //############


    EPwm3Regs.TBCTL.bit.CLKDIV =  2;    // CLKDIV = 4
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
    EPwm3Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
    EPwm3Regs.TBPRD  = EPwm1Regs.TBPRD;


    EPwm3Regs.CMPA.half.CMPA  = EPwm1Regs.CMPA.half.CMPA ;
    EPwm3Regs.AQCTLA.all = 0x0060;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = 0;  // ucitaj kada je CMP = 0;
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = 0; // Shadow mode enabled;



    // pitanje mrtvog vremena
    // delay = Tsystem( 1 / 150Mhz)*HSPCLKDIV*CLKDIV ( = 4) * DBRED  //

    EPwm1Regs.DBCTL.all = 0x000B; // ACTIVE HIGH COMPLEMENTARY ???
    EPwm1Regs.DBRED = 75; // postavi kasnjenje za rising edge ( ovo je 2.67microsec )
    EPwm1Regs.DBFED = 75; // postavi kasnjenje za falling edge ( strana 42 )

    EPwm2Regs.DBCTL.all = 0x000B;
    EPwm2Regs.DBRED = 75;
    EPwm2Regs.DBFED = 75;

    EPwm3Regs.DBCTL.all = 0x000B;
    EPwm3Regs.DBRED = 75;
    EPwm3Regs.DBFED = 75;


}

interrupt void ePWM1A_compare_isr(void)
{

    EALLOW;
    SysCtrlRegs.WDKEY = 0xAA;   // service WD #2
    EDIS;

    // enable

    GpioDataRegs.GPASET.bit.GPIO16 = ResetFLT != 0 ; // Nema upisa u CLEAR registar jer ovaj pin
                                                             // ne treba da se rucno clear-uje (osim na pocetku kada je na nuli)
                                                             // to ce se desiti u slucaju da dodje do kvara
    GpioDataRegs.GPASET.bit.GPIO6 = Enable != 0 ;
    GpioDataRegs.GPACLEAR.bit.GPIO7 = Enable != 0 ;
    GpioDataRegs.GPACLEAR.bit.GPIO6 = !(Enable!=0);
    GpioDataRegs.GPASET.bit.GPIO7 = !(Enable!=0);

    m = frequency/50; //koren_iz_tri*Vref/Voltage_DC;


    if (frequency_new > frequency) {
                    frequency += slope*EPwm1Regs.TBPRD*TCLK;
                    if (frequency > frequency_new) frequency = frequency_new;
    }
    if (frequency_new < frequency) {
                    frequency -= slope*EPwm1Regs.TBPRD*TCLK;
                    if (frequency < frequency_new) frequency = frequency_new;
    }




    EPwm1Regs.CMPA.half.CMPA  = (int) ((EPwm1Regs.TBPRD * (1-((2/koren_iz_tri)*m*(sin(Theta)+sestina*sin(3*Theta))))/2));
    EPwm2Regs.CMPA.half.CMPA  = (int) ((EPwm1Regs.TBPRD * (1-((2/koren_iz_tri)*m*(sin(Theta+dva_pi_trecina)+sestina*sin(3*Theta))))/2));
    EPwm3Regs.CMPA.half.CMPA  = (int) ((EPwm3Regs.TBPRD * (1-((2/koren_iz_tri)*m*(sin(Theta-dva_pi_trecina)+sestina*sin(3*Theta))))/2));




    Theta_increment = dva_pi*frequency/fpwm; // dva_pi*frequency*Tpwm, ova rutina se izvrsava svakih Tpwm
    Theta = Theta + Theta_increment;

    if ( Theta >= dva_pi )
    {
        Theta = Theta - dva_pi;
    }

    EPwm1Regs.ETCLR.bit.INT = 1; //clear interrupt flag
    PieCtrlRegs.PIEACK.all = 4; //acknowledge interrupt

}
