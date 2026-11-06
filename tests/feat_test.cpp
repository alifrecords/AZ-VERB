#include "../Source/dsp/AZVerbEngine.h"
#include <cstdio>
// "Vokal" imitatsiyasi: 0.6 s ovoz (garmonikalar + 7 kHz "s" shovqini), 0.6 s jimlik
int main(){ const int fs=48000; az::Rng r;
 auto run=[&](bool duck,float focus,float deEss,double& wetSing,double& wetGap,double& sibWet){
  az::Engine e; e.prepare(fs,512); az::Params p; p.algo=az::Algo::Plate; p.mix=1; p.duck=duck; p.focus=focus; p.deEss=deEss; p.crystal=0.4f; e.setParams(p); e.reset();
  const int n=fs*6; std::vector<float> L(n),R(n); for(int i=0;i<n;++i){ double t=(double)i/fs; bool on=fmod(t,1.2)<0.6; float v=0;
   if(on){ for(int h=1;h<8;++h) v+=0.12f/h*sin(2*M_PI*180*h*t); if(fmod(t,1.2)<0.08) v+=0.25f*r.next(); } L[i]=R[i]=v; }
  std::vector<float> in=L; for(int i=0;i<n;i+=512) e.process(L.data()+i,R.data()+i,std::min(512,n-i));
  wetSing=wetGap=0; int a=0,b=0; for(int i=fs*2;i<n;++i){ double t=(double)i/fs; bool on=fmod(t,1.2)<0.6; if(on){wetSing+=L[i]*L[i];a++;} else {wetGap+=L[i]*L[i];b++;} }
  wetSing=10*log10(wetSing/a); wetGap=10*log10(wetGap/b);
  az::SVF hp; hp.set(6000,0.7f,fs); sibWet=0; for(int i=fs*2;i<n;++i){ float h=hp.hp(L[i]); sibWet+=h*h;} sibWet=10*log10(sibWet/(n-fs*2)); };
 double s,g,sb;
 run(false,0,0,s,g,sb); printf("Hammasi o'chiq  : kuylash=%.1f dB  pauza=%.1f dB  sibilyant=%.1f dB\n",s,g,sb);
 run(true,0,0,s,g,sb);  printf("DUCK yoniq      : kuylash=%.1f dB  pauza=%.1f dB\n",s,g);
 run(true,1,0,s,g,sb);  printf("DUCK+FOCUS 100%% : kuylash=%.1f dB  pauza=%.1f dB\n",s,g);
 run(false,0,1,s,g,sb); printf("DE-ESS 100%%     : sibilyant=%.1f dB\n",sb);
}
