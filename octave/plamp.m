% Copyright David Rowe 2009
% This program is distributed under the terms of the GNU General Public License 
% Version 2

function plamp(samname, f)
  
  sn_name = strcat(samname,"_sn.txt");
  Sn = load(sn_name);
  model_name = strcat(samname,"_model.txt");
  model = load(model_name);
  modelq_name = strcat(samname,"_qmodel.txt");
  modelq = load(modelq_name);
  pw_name = strcat(samname,"_pw.txt");
  Pw = load(pw_name);

  figure(1);
  clg;
  s = [ Sn(2*f+1,:) Sn(2*f+2,:) ];
  plot(s);
 
  figure(2);
  Wo = model(f,1);
  L = model(f,2);
  Am = model(f,3:(L+2));
  plot((1:L)*Wo*4000/pi, 20*log10(Am),";Am;");
  hold on;
  Amq = modelq(f,3:(L+2));
  plot((1:L)*Wo*4000/pi, 20*log10(Amq),";Amq;" );
  plot((0:255)*4000/256, 10*log10(Pw(f,:)),";Pw;");
  plot((1:L)*Wo*4000/pi, 20*log10(Amq) - 20*log10(Am),";Am_err;" );
  hold off;

  Am_err = mean(abs(20*log10(Amq) - 20*log10(Am)))
endfunction
