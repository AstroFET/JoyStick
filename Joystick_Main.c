
#include "driverlib.h"
#include <msp430.h>
#include <stdint.h>
#include "stdio.h"
#include <EEL4746.h>

#define   Num_of_Results   1

void myDelay(uint32_t);

const uint32_t LOAD = 104500;
const uint8_t MaxCount = 7;
const uint8_t MinCount = 0;
int8_t vSeq = 0;
volatile uint16_t AXresults;
volatile uint16_t AYresults;


void main(void)

{

    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    P2DIR |= BIT4;                              //RED LED MKII
    P1DIR |= BIT5;                              //GREEN LED MKII
    P1DIR |= BIT0;                              //RED LED MP430
    P4DIR |= BIT7;                              //GREEN LED MP430

    P2OUT |= BIT4;                              //LEDs made as outputs and initially turned on
    P1OUT |= BIT5;
    P1OUT |= BIT0;
    P4OUT |= BIT7;

    myDelay(LOAD);

    P2OUT &= ~BIT4;
    P1OUT &= ~BIT5;
    P1OUT &= ~BIT0;
    P4OUT &= ~BIT7;

    //Enable A/D channel inputs
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN3 + GPIO_PIN5
                                               );

    //Initialize the ADC12_A Module
    /*
     * Base address of ADC12_A Module
     * Use internal ADC12_A bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC12_A_init(ADC12_A_BASE,
                 ADC12_A_SAMPLEHOLDSOURCE_SC,
                 ADC12_A_CLOCKSOURCE_ADC12OSC,
                 ADC12_A_CLOCKDIVIDER_1
                 );

    ADC12_A_enable(ADC12_A_BASE);

    /*
     * Base address of ADC12_A Module
     * For memory buffers 0-7 sample/hold for 256 clock cycles
     * For memory buffers 8-15 sample/hold for 4 clock cycles (default)
     * Enable Multiple Sampling
     */
    ADC12_A_setupSamplingTimer(ADC12_A_BASE,
                               ADC12_A_CYCLEHOLD_256_CYCLES,
                               ADC12_A_CYCLEHOLD_4_CYCLES,
                               ADC12_A_MULTIPLESAMPLESENABLE);

    //Configure Memory Buffers
    /*
     * Base address of the ADC12_A Module
     * Configure memory buffer 0
     * Map input A3 to memory buffer 0
     * Vref+ = AVcc
     * Vref- = AVss
     * Memory buffer 0 is not the end of a sequence
     */
    ADC12_A_configureMemoryParam param0 = {0};
    param0.memoryBufferControlIndex = ADC12_A_MEMORY_0;
    param0.inputSourceSelect = ADC12_A_INPUT_A3;
    param0.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
    param0.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
    param0.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;
    ADC12_A_configureMemory(ADC12_A_BASE,&param0);

    /*
     * Base address of the ADC12_A Module
     * Configure memory buffer 1
     * Map input A1 to memory buffer 1
     * Vref+ = AVcc
     * Vref- = AVss
     * Memory buffer 1 is the end of a sequence
     *
     */
    ADC12_A_configureMemoryParam param1 = {0};
    param1.memoryBufferControlIndex = ADC12_A_MEMORY_1;
    param1.inputSourceSelect = ADC12_A_INPUT_A5;
    param1.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
    param1.negativeRefVoltageSourceSelect = ADC12_A_VREFNEG_AVSS;
    param1.endOfSequence = ADC12_A_ENDOFSEQUENCE;
    ADC12_A_configureMemory(ADC12_A_BASE,&param1);


    //Enable memory buffer 1 interrupt
    ADC12_A_clearInterrupt(ADC12_A_BASE,
                           ADC12IFG1);
    ADC12_A_enableInterrupt(ADC12_A_BASE,
                            ADC12IE1);

    //Enable/Start first sampling and conversion cycle
    /*
     * Base address of ADC12_A Module
     * Start the conversion into memory buffer 0
     * Use the repeated sequence of channels
     */
    ADC12_A_startConversion(ADC12_A_BASE,
                            ADC12_A_MEMORY_0,
                            ADC12_A_REPEATED_SEQOFCHANNELS);

    //Enable interrupts
    __bis_SR_register(GIE);


    // Enter your main C code here



    //Repeat -- Until Loop
    while(1){
    // Y axis is recorded in variable AYResult , loop where new code was add

        ASMMotorDriver(vSeq);
        myDelay(LOAD);


        if(AYresults > 2291)
        {
            vSeq++;
            if(vSeq > MaxCount)
                vSeq = 0;
            if(vSeq < MinCount)
                vSeq = 7;
        }

        if(AYresults < 1875)
        {
            vSeq--;
            if(vSeq > MaxCount)
                vSeq = 0;
            if(vSeq < MinCount)
                vSeq = 7;
        }

    __no_operation();
    }
}



#pragma vector=ADC12_VECTOR
__interrupt
void ADC12ISR(void)
{
//    static uint16_t index = 0;

    switch(__even_in_range(ADC12IV,34))
    {
    case  0: break;       //Vector  0:  No interrupt
    case  2: break;       //Vector  2:  ADC overflow
    case  4: break;       //Vector  4:  ADC timing overflow
    case  6: break;       //Vector  6:  ADC12IFG0
    case  8:               //Vector  8:  ADC12IFG1
         //Move A3 results, IFG is cleared
        AYresults =
            ADC12_A_getResults(ADC12_A_BASE,
                               ADC12_A_MEMORY_0);
        //Move A5 results, IFG is cleared
        AXresults =
            ADC12_A_getResults(ADC12_A_BASE,
                               ADC12_A_MEMORY_1);
        //Set BREAKPOINT here
        __no_operation();
        break;

    case 10: break;       //Vector 10:  ADC12IFG2
    case 12: break;       //Vector 12:  ADC12IFG3
    case 14: break;       //Vector 14:  ADC12IFG4
    case 16: break;       //Vector 16:  ADC12IFG5
    case 18: break;       //Vector 18:  ADC12IFG6
    case 20: break;       //Vector 20:  ADC12IFG7
    case 22: break;       //Vector 22:  ADC12IFG8
    case 24: break;       //Vector 24:  ADC12IFG9
    case 26: break;       //Vector 26:  ADC12IFG10
    case 28: break;       //Vector 28:  ADC12IFG11
    case 30: break;       //Vector 30:  ADC12IFG12
    case 32: break;       //Vector 32:  ADC12IFG13
    case 34: break;       //Vector 34:  ADC12IFG14
    default: break;
    }
}

void myDelay(uint32_t LOAD)
{
    EnableTimer4746(LOAD);

    while ((TA0CTL & BIT0) == 0){}

    DisableTimer4746();
}
