# KETACLOCK clone

����� MIRO/MobileHackerz ����̃v���W�F�N�g[KETACLOCK - �����ƂɓƗ��������v](http://mobilehackerz.jp/contents/Hardware/KETACLOCK)��͕킵�����̂ł��B���ۂ̉�H��R�[�h�͌��J����Ă��Ȃ����߃I���W�i���Ƃ͈قȂ�܂��B

* �������𑗐M������̃}�X�^�Ɗe���̕\���������Ȃ��X���[�u���j�b�g����Ȃ�B
* �ł��邾���d�r�œ��삳����B
* �}�X�^���j�b�g�ɂ� ESP8266 ��p���� NTP �����ɂ�鐳�m�Ȏ�������������B
* �w�K�̂��߃X���[�u���j�b�g�͈قȂ�A�[�L�e�N�`���̃}�C�R�����g���Ă݂�B
* �}�X�^ - �X���[�u�̒ʐM�ɂ͐ԊO���ʐM(NEC�t�H�[�}�b�g)���̗p����B

## �ԊO���ʐM

�}�X�^����X���[�u�ɂ͐ԊO���ʐM�ɂ�莞��(�����b)���𑗐M���܂��B
UART �f�[�^�𒼐ڈ������Ƃ��l���܂������A�����̃��C�u�������g���ĊȒP�ɓ��������ƐԊO�������R���ɗp������ NEC �t�H�[�}�b�g���̗p���܂����B���\�Ɏ����b�f�[�^�����̂悤�Ɋ���U���Ă��܂��B���̂��߂Ɉ�ʂ̃����R���Ɗ����܂��B���[�_�[����ύX����ȂǓƎ��t�H�[�}�b�g�Ƃ��ׂ��ł��傤�������ړI�Ƃ��Ėڂ��Ԃ��Ă��܂��BUART �f�[�^���ڂł��ǂ����Ǝv���܂��B

* ��:�A�h���X���8�r�b�g
* ��:�A�h���X����8�r�b�g
* �b:�R�}���h8�r�b�g

## ESP8266 - master

WiFi �ɐڑ��� NTP �T�[�o�Ɠ������Ď��������擾�AOLED �ɕ\�����Ė��b��������ԊO�����M���܂��B

* �ԊO���ʐM���C�u���� https://github.com/Arduino-IRremote/Arduino-IRremote  
�g�p�s���� PinDefinitainAndMore.h �� ESP8266 �Ɋւ��镔�����Q��
* I2C �ڑ� OLED (SSD1306 128x64) Adafruit_SSD1306 ���C�u����  

## AVR - slave

ATmega328P ���O�t���������U��Ȃ��� 3.3V/8MHz ����Ƃ��� [Arduino](https://github.com/technoblogy/atmegabreadboard) �œ������Ă��܂��B�莝���̊֌W�� 328P ���g���܂����� 168(P) �ł������ł��傤�B

IR �ʐM�ɂ� ESP8266 �Ɠ����� Arduino-IRremote ���C�u������p���Ă��܂��B7-seg LCD �� GPIO �ɂ�钼�ڋ쓮�ŏĕt���h�~�̂��� 50Hz �ŋɐ����]�������Ȃ��܂��B�s���z�u�ɂ��Ă̓\�[�X���̃R�����g���Q�Ƃ��Ă��������B���̃X���[�u���j�b�g�����l�ł��B

## MSP430 - slave

MSP430G2553 �� Energia �œ������Ă��܂��B�O�t���������U��͎g���܂���B
Energia �� IRremote ���C�u������ Arduino �̂��̂Ƃ͈قȂ�܂��B

## CH32V - slave

������ RISCV �}�C�R�� CH32V003F4P6 ���O�t���������U�햳�� xxMHz �œ������Ă��܂��B
������� Arduino ���ł̃��C�u�������s�\���Ȃ��߁AMounRiver Studio ���Ńr���h���܂����B

�ԊO����M�ɂ��Ă� https://github.com/openwch/ch32v003 ���� InputCapture �̃R�[�h���Q�l�ɏ����Ă��܂��B

![KETACLOCK](KETACLOCK.jfif)

