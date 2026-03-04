clear all; close all;

% RLC meter values
Z_RC_RLCMeter  = [15170 14870 13470 12720 9986 7566 5996 4935 4181 3622 ...
                  3192 2852 2577 1735 1307 1049 875.7 658.3 527.5 440.2 ...
                  377.8 330.9 295.5 265.3 133.9 27.79];
Phase_RLCMeter = [-17.2 -20.3 -31.4 -36.1 -50.2 -60.6 -66.9 -70.9 -73.7 ... 
                  -75.8 -77.4 -78.6 -79.6 -82.6 -84.2 -85.1 -85.7 -86.5 ...
                  -87.0 -87.3 -87.5 -87.7 -87.8 -87.9 -88.4 -89.0];
Freq_RLCMeter  = [50 60 100 120 200 300 400 500 600 700 800 900 1000 ... 
                  1500 2000 2500 3000 4000 5000 6000 7000 8000 9000  ... 
                  10000 20000 100000];
             
% AD5940 values
Z_RC_AD5940  = [17986 18911 17783 17701 17302 16510 15571 14583 13799 ...
                12902 11923 11412 10390 7856 6078 4949 4295 3218 2580 ...
                2222 1913 1647 1417 1314 901 666 422 335 267 212 183 158 ...
                148 139];
    % Measured Phase with 2 value with wrong sign (the 2 272°)
Phase_AD5940 = [-2.8 -6.4 -7.4 -9.4 -14.7 -22.5 -29.3 -35.1 -39.2 -43.5 ...
                -47.8 -49.9 -54.1 -63.5 -69.6 -73.4 -75.5 -79.1 -81.1 ...
                -82.3 -83.3 -84.1 -84.9 -85.2 -86.5 -87.2 -87.9 -88.1 ...
                -88.3 -88.4 272.0 272.7 -86.8 -86.1];
    % Measured Phase with the 2 value "corrected"
Phase_AD5940 = [-2.8 -6.4 -7.4 -9.4 -14.7 -22.5 -29.3 -35.1 -39.2 -43.5 ...
                -47.8 -49.9 -54.1 -63.5 -69.6 -73.4 -75.5 -79.1 -81.1 ...
                -82.3 -83.3 -84.1 -84.9 -85.2 -86.5 -87.2 -87.9 -88.1 ...
                -88.3 -88.4 -88.0 -87.3 -86.8 -86.1];
            
Freq_AD5940  = [50 62 99 124 196 309 420 527 614 715 832 898 1045 ... 
                1529 2072 2603 3030 4107 5159 6006 6992 8140 9477  ... 
                10225 14953 20266 31977 40167 50455 63377 73783 85897 ...
                92680 100000];
             
Z_RC = [];
Z_RC_AD5940_relerr = [];
Phase_AD5940_relerr = [];

R = 18e3;   % [Ohm]
C = 10e-9;  % [F]
f_start = 50;    % [Hz]
f_stop = 100e3;  % [Hz]

for f = f_start:1:f_stop
    w = 2*pi*f;
    
    Z_RC(f-f_start+1) = 1/sqrt((1/R)^2 + (w*C)^2);
    
    Phase(f-f_start+1) = atan(-w*C*R) * 180/pi;
end

 for  n = 1:length(Freq_AD5940)
     Z_RC_AD5940_relerr(n) = abs((Z_RC(Freq_AD5940(n)-f_start+1)-Z_RC_AD5940(n))/Z_RC(Freq_AD5940(n)-f_start+1)) ;  % relative error
     Phase_AD5940_relerr(n) = abs((Phase(Freq_AD5940(n)-f_start+1)-Phase_AD5940(n))/Phase(Freq_AD5940(n)-f_start+1)) ;  % relative error
 end

f_plot = [f_start:1:f_stop];

%tiledlayout(2,1);

%nexttile;
yyaxis left
plot(f_plot, Z_RC,'LineWidth',2)
xlabel('f [Hz]')
ylabel('Magnitude [Ohm]')

%nexttile;
yyaxis right
plot(f_plot, Phase, '--', 'LineWidth',2)
title('Magnitude & Phase')
xlabel('f [Hz]')
ylabel('\Phi [Deg]')

hold on;

% yyaxis left
% plot(Freq_RLCMeter, Z_RC_RLCMeter, '-m', 'LineWidth',2)
% xlabel('f [Hz]')
% ylabel('Magnitude [Ohm]')
% 
% %nexttile;
% yyaxis right
% plot(Freq_RLCMeter, Phase_RLCMeter, '--c', 'LineWidth',2)
% xlabel('f [Hz]')
% ylabel('\Phi [Deg]')
% 
yyaxis left
errorbar(Freq_AD5940, Z_RC_AD5940, Z_RC_AD5940_relerr, '-b', 'LineWidth',2)
xlabel('f [Hz]')
ylabel('Magnitude [Ohm]')

%nexttile;
yyaxis right
errorbar(Freq_AD5940, Phase_AD5940, Phase_AD5940_relerr, '--g', 'LineWidth',2)
xlabel('f [Hz]')
ylabel('\Phi [Deg]')

legend('Z\_RC\_Theo', 'Z\_RC\_AD5940', 'Phase\_Theo', ...
       'Phase\_AD5940', 'Location','NorthEastOutside')

grid on;

figure()
yyaxis left
scatter(Freq_AD5940, Z_RC_AD5940_relerr*100, 'xr');
xlabel('f [Hz]')
ylabel('Magnitude error [%]')
yyaxis right
scatter(Freq_AD5940, Phase_AD5940_relerr*100, '*b');
xlabel('f [Hz]')
ylabel('\Phi  error [%]')

title('Magnitude & Phase Error')
grid on;

legend('Z\_RC\_AD5940\_error', 'Phase\_AD5940\_error', 'Location','NorthEastOutside')
