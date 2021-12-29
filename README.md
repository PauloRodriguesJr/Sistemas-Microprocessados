# Sistemas-Microprocessados

Este repositório trata da Disciplina Sistemas Microprocessados da Universidade Federal do UFABC, cursada no 3º quadrimestre de 2021.

## Projeto Final

O projeto final da disciplina consistiu em fazer um conjunto de tarefas operadas por um microcrontrolador STM32F103C8 associado a um flow do NodeRED. O sistema possuía as seguintes funcionalidades:

A) Um conjunto de LEDs que piscam em dois padrões, cujo chavemento dos modos ocorresse por uma interrupção gerada por software:
  1-  D1=200, 
      D2=300,
      D3=400, 
      D4=500 ms.
  2 - piscar todos juntos: DT = 500ms.
  
B) Gráfico apresentando os 4 LEDs e o pulso de Interrupção.

C) Um medidor de PWM no CubeMonitor com resolução do PWM de 200; 
   Intervalo entre 0 e MAX do PWM: 3 seg.

D) Um TIMER apresentado em TEXT/7-SEG e cronômetro crescente, com resolução de décimos de segundo;
   Funções do cronometro implementadas:
   1 - Botão de iniciar / pausar.
   2 - Botão para limpar cronômetro.
   
   
## Resultados

A animação abaixo apresenta as funcionalidades implementadas em um dashboard NodeRED, utilizando o software CubeMonitor.

<p align="center">
<img src="docs/project_demonstration.gif" width="600"/>
</p>
