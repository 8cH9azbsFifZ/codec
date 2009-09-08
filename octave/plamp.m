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

  sw__name = strcat(samname,"_sw_.txt");
  if (file_in_path(".",sw__name))
    Sw_ = load(sw__name);
  endif

  model_name = strcat(samname,"_model.txt");
  model = load(model_name);

  modelq_name = strcat(samname,"_qmodel.txt");
  if (file_in_path(".",modelq_name))
    modelq = load(modelq_name);
  endif

  pw_name = strcat(samname,"_pw.txt");
  if (file_in_path(".",pw_name))
    Pw = load(pw_name);
  endif

  lsp_name = strcat(samname,"_lsp.txt");
  if (file_in_path(".",lsp_name))
    lsp = load(lsp_name);
  endif

  phase_name = strcat(samname,"_phase.txt");
  if (file_in_path(".",phase_name))
    phase = load(phase_name);
  endif

  phase_name_ = strcat(samname,"_phase_.txt");
  if (file_in_path(".",phase_name_))
    phase_ = load(phase_name_);
  endif

  do 
    figure(1);
    clg;
    s = [ Sn(2*f-1,:) Sn(2*f,:) ];
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

    if (file_in_path(".",modelq_name))
      Amq = modelq(f,3:(L+2));
      plot((1:L)*Wo*4000/pi, 20*log10(Amq),";Amq;" );
      if (file_in_path(".",pw_name))
        plot((0:255)*4000/256, 10*log10(Pw(f,:)),";Pw;");
      endif	
      signal = Am * Am';
      noise = (Am-Amq) * (Am-Amq)'; 
      snr = 10*log10(signal/noise);
      Am_err_label = sprintf(";Am_err SNR %4.2f dB;",snr);
      plot((1:L)*Wo*4000/pi, 20*log10(Amq) - 20*log10(Am), Am_err_label);
    endif

    % phase model - determine SNR and error spectrum

    if (file_in_path(".",phase_name_))
      orig  = Am.*exp(j*phase(f,1:L));
      synth = Am.*exp(j*phase_(f,1:L));
      signal = orig * orig'
      noise = (orig-synth) * (orig-synth)'
      snr = 10*log10(signal/noise);

      phase_err_label = sprintf(";phase_err SNR %4.2f dB;",snr);
      plot((1:L)*Wo*4000/pi, 20*log10(orig-synth), phase_err_label);
    endif

    if (file_in_path(".",lsp_name))
      for l=1:10
        plot([lsp(f,l)*4000/pi lsp(f,l)*4000/pi], [60 80], 'r');
      endfor
    endif

    hold off;

    if (file_in_path(".",phase_name))
      figure(3);
      plot((1:L)*Wo*4000/pi, unwrap(phase(f,1:L)), ";phase;");
      axis;
      if (file_in_path(".",phase_name_))
        hold on;
        plot((1:L)*Wo*4000/pi, unwrap(phase_(f,1:L)), ";phase_;");
	hold off;
      endif
      figure(2);
    endif

    % interactive menu

    printf("\rframe: %d  menu: n-next  b-back  p-png  q-quit ", f);
    fflush(stdout);
    k = kbhit();
    if (k == 'n')
      f = f + 1;
    endif
    if (k == 'b')
      f = f - 1;
    endif

    % optional print to PNG

    if (k == 'p')
    
      pngname = sprintf("%s_%d",samname,f);

      % small image

      __gnuplot_set__ terminal png size 420,300
      ss = sprintf("__gnuplot_set__ output \"%s.png\"", pngname);
      eval(ss)
      replot;

      % larger image

      __gnuplot_set__ terminal png size 800,600
      ss = sprintf("__gnuplot_set__ output \"%s_large.png\"", pngname);
      eval(ss)
      replot;

      % for some reason I need this to stop large plot getting wiped
      __gnuplot_set__ output "/dev/null"

    endif

  until (k == 'q')
  printf("\n");

endfunction
