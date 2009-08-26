% Copyright David Rowe 2009
% This program is distributed under the terms of the GNU General Public License 
% Version 2
%
% Plot ampltiude modelling information from dump files.

function plamp(samname, f)
  
  sn_name = strcat(samname,"_sn.txt");
  Sn = load(sn_name);
  sw_name = strcat(samname,"_sw.txt");
  Sw = load(sw_name);
  model_name = strcat(samname,"_model.txt");
  model = load(model_name);
  modelq_name = strcat(samname,"_qmodel.txt");
  modelq = load(modelq_name);
  pw_name = strcat(samname,"_pw.txt");
  Pw = load(pw_name);

  do 
    figure(1);
    clg;
    s = [ Sn(2*f+1,:) Sn(2*f+2,:) ];
    plot(s);
    axis([1 length(s) -20000 20000]);

    figure(2);
    Wo = model(f,1);
    L = model(f,2);
    Am = model(f,3:(L+2));
    plot((1:L)*Wo*4000/pi, 20*log10(Am),";Am;");
    axis([1 4000 -10 80]);
    hold on;
    plot((0:255)*4000/256, Sw(f,:),";Sw;");
    Amq = modelq(f,3:(L+2));
    plot((1:L)*Wo*4000/pi, 20*log10(Amq),";Amq;" );
    plot((0:255)*4000/256, 10*log10(Pw(f,:)),";Pw;");
    signal = Am * Am';
    noise = (Am-Amq) * (Am-Amq)'; 
    snr = 10*log10(signal/noise);
    Am_err_label = sprintf(";Am_err SNR %4.2f dB;",snr);
    %plot((1:L)*Wo*4000/pi, 20*log10(Amq) - 20*log10(Am),";Am_err;" );
    plot((1:L)*Wo*4000/pi, 20*log10(Amq) - 20*log10(Am), Am_err_label);
    hold off;

    printf("\rframe: %d  menu: n-next  b-back  q-quit ", f);
    fflush(stdout);
    k = kbhit();
    if (k == 'n')
      f = f + 1;
    endif
    if (k == 'b')
      f = f - 1;
    endif
  until (k == 'q')
  printf("\n");

endfunction
