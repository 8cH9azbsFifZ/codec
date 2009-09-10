% Copyright David Rowe 2009
% This program is distributed under the terms of the GNU General Public License 
% Version 2
%
% Plot phase modelling information from dump files.

function plphase(samname, f)
  
  sn_name = strcat(samname,"_sn.txt");
  Sn = load(sn_name);

  sw_name = strcat(samname,"_sw.txt");
  Sw = load(sw_name);

  model_name = strcat(samname,"_model.txt");
  model = load(model_name);

  pw_name = strcat(samname,"_pw.txt");
  if (file_in_path(".",pw_name))
    Pw = load(pw_name);
  endif

  ak_name = strcat(samname,"_ak.txt");
  if (file_in_path(".",ak_name))
    ak = load(ak_name);
  endif

  phase_name = strcat(samname,"_phase.txt");
  if (file_in_path(".",phase_name))
    phase = load(phase_name);
  endif

  phase_name_ = strcat(samname,"_phase_.txt");
  if (file_in_path(".",phase_name_))
    phase_ = load(phase_name_);
  endif

  snr_name = strcat(samname,"_snr.txt");
  if (file_in_path(".",snr_name))
    snr = load(snr_name);
  endif

  sn_name_ = strcat(samname,".raw");
  if (file_in_path(".",sn_name_))
    fs_ = fopen(sn_name_,"rb");
    sn_  = fread(fs_,Inf,"short");
  endif

  do 
    figure(1);
    clg;
    s = [ Sn(2*f-1,:) Sn(2*f,:) ];
    plot(s);
    grid;
    axis([1 length(s) -20000 20000]);

    figure(2);
    Wo = model(f,1);
    L = model(f,2);
    Am = model(f,3:(L+2));
    plot((1:L)*Wo*4000/pi, 20*log10(Am),";Am;");
    axis([1 4000 -10 80]);
    hold on;
    plot((0:255)*4000/256, Sw(f,:),";Sw;");
    grid;

    if (file_in_path(".",pw_name))
       plot((0:255)*4000/256, 10*log10(Pw(f,:)),";Pw;");
    endif	

    if (file_in_path(".",snr_name))
      snr_label = sprintf(";phase SNR %4.2f dB;",snr(f));
      plot(1,1,snr_label);
    endif

    % phase model - determine SNR and error spectrum for phase model 1

    if (file_in_path(".",phase_name_))
      orig  = Am.*exp(j*phase(f,1:L));
      synth = Am.*exp(j*phase_(f,1:L));
      signal = orig * orig';
      noise = (orig-synth) * (orig-synth)';
      snr_phase = 10*log10(signal/noise);

      phase_err_label = sprintf(";phase_err SNR %4.2f dB;",snr_phase);
      plot((1:L)*Wo*4000/pi, 20*log10(orig-synth), phase_err_label);
      hold off;
    endif

    if (file_in_path(".",phase_name))
      figure(3);
      plot((1:L)*Wo*4000/pi, phase(f,1:L)*180/pi, "-o;phase;");
      axis;
      if (file_in_path(".", phase_name_))
        hold on;
        plot((1:L)*Wo*4000/pi, phase_(f,1:L)*180/pi, ";phase_;");
	grid
	hold off;
      endif
    endif

    % synthesised speech 

    if (file_in_path(".",sn_name_))
      figure(4);
      s_ = sn_((f-3)*80+1:(f+1)*80);
      plot(s_);
      axis([1 length(s_) -20000 20000]);
    endif

    if (file_in_path(".",ak_name))
      figure(5);
      axis;
      akw = ak(f,:);
      weight = 1.0 .^ (0:length(akw)-1);
      akw = akw .* weight;
      H = 1./fft(akw,8000);
      subplot(211);
      plot(20*log10(abs(H(1:4000))),";LPC mag spec;");
      grid;	
      subplot(212);
      plot(angle(H(1:4000))*180/pi,";LPC phase spec;");
      grid;
    endif

    hold off;

    % autocorrelation function to research voicing est
    
    %M = length(s);
    %sw = s .* hanning(M)';
    %for k=0:159
    %  R(k+1) = sw(1:320-k) * sw(1+k:320)';
    %endfor
    %figure(4);
    %R_label = sprintf(";R(k) %3.2f;",max(R(20:159))/R(1));
    %plot(R/R(1),R_label);
    %grid

    figure(2);

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
