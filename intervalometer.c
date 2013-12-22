#ifdef __MSP430G2452__
#  include <msp430g2452.h>
#endif
#ifdef __MSP430G2231__
#  include <msp430g2231.h>
#endif
#include <legacymsp430.h>


#if __GNUC__ > 4 || \
  (__GNUC__ == 4 && (__GNUC_MINOR__ > 5 || \
    (__GNUC_MINOR__ == 5 && \
     __GNUC_PATCHLEVEL__ > 2)))
#else
#  error We need msp430-gcc >= 4.5.3
#endif

int g_timerInterval;
int g_shutterOpen;

#define SHUTTER_LENGTH 100

int main(void) {

  // Stop WDT

  WDTCTL = WDTPW + WDTHOLD; 

  BCSCTL1 |= DIVA_3;				// ACLK/8
  BCSCTL3 |= XCAP_3;				//12.5pF cap- setting for 32768Hz crystal
  
  // Set P1.0 (LED1) as output, and set it low
 
  P1DIR |= BIT0;
  P1OUT &= ~BIT0;

  // Set bits 1-4 as inputs, with pullup resistors enabled

  P1DIR &= ~(BIT1 + BIT2 + BIT3 + BIT4);
  P1REN |= BIT1 + BIT2 + BIT3 + BIT4;
  P1OUT |= BIT1 + BIT2 + BIT3 + BIT4;

  // Read in the value of the DIP switches

  int switchValue = (P1IN & 0x1e) >> 1;
  g_shutterOpen = 0;
  g_timerInterval = (switchValue + 1) * 511;

  // Enable the CCR0 interrupt, set the timer interval, and ACLK	
  
  CCTL0 = CCIE;
  CCR0 = g_timerInterval;  
  TACTL = TASSEL_1 + ID_3 + MC_1;

  // Go to LPM3 and await interrupts

  __bis_SR_register(LPM3_bits | GIE);

  return 0;
}

// Timer ISR

#ifdef __MSP430G2452__
interrupt(TIMER0_A0_VECTOR) ta1_isr(void)
#endif
#ifdef __MSP430G2231__
interrupt(TIMERA0_VECTOR) ta1_isr(void)
#endif
{

  if (g_shutterOpen) {

	// The shutter is open, close it and wait for the next interval
	
	P1OUT &= ~BIT0;
	CCR0 = g_timerInterval;
	g_shutterOpen = 0;
  } else {

	// The shutter is closed, open it and wait for SHUTTER_LENGTH

	P1OUT |= BIT0;
	CCR0 = SHUTTER_LENGTH;
	g_shutterOpen = 1;
  }
  
}


// ADC ISR

interrupt(ADC10_VECTOR) adc_isr(void) {
  
  // Measurement complete, turn the CPU back on

  __bic_SR_register_on_exit(CPUOFF);

}

