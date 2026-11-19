#pragma once
#include <QColor>
#include <QImage>
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

struct CoverPalette {
 QColor primary{"#527a9c"},secondary{"#75658f"},deep{"#101c29"},accent{"#b89a67"};
 double monochromeStrength=0;

 static CoverPalette fromImage(const QImage &cover) {
  if(cover.isNull())return {};
  constexpr int hueCount=48;
  struct Hue {
   int pixels=0;
   double weight=0,sin=0,cos=0,light=0,saturation=0;
   void add(const Hue &other){
    pixels+=other.pixels;weight+=other.weight;sin+=other.sin;cos+=other.cos;
    light+=other.light;saturation+=other.saturation;
   }
   double angle()const{return std::fmod(std::atan2(sin,cos)/(2.*std::numbers::pi)+1.,1.);}
  };
  std::array<Hue,hueCount> histogram{};
  const auto image=cover.scaled(96,96,Qt::KeepAspectRatio,Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
  int total=0,chromatic=0;
  double meanLight=0;
  for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x){
   const auto pixel=image.pixel(x,y);
   if(qAlpha(pixel)<192)continue;
   const QColor color(pixel);
   const double light=color.lightnessF(),sat=color.hslSaturationF(),hue=color.hslHueF();
   const double chroma=(1.-std::abs(2.*light-1.))*sat;
   ++total;meanLight+=light;
   // Deep navy supplies the base, but a dark pixel's high HSL saturation
   // alone should never outweigh a visible sunset or nebula.
   if(hue<0||chroma<.021||light<.024||light>.96)continue;
   ++chromatic;
   const double weight=std::sqrt(chroma)*(.25+.75*std::sqrt(light));
   const double angle=hue*2.*std::numbers::pi;
   histogram[std::min(hueCount-1,int(hue*hueCount))].add({1,weight,weight*std::sin(angle),weight*std::cos(angle),weight*light,weight*sat});
  }
  if(!total)return {};
  meanLight/=total;
  auto neutralPalette=[meanLight]{
   auto gray=[](double v){return QColor::fromRgbF(v,v,v);};
   return CoverPalette{gray(.31+meanLight*.26),gray(.24+meanLight*.26),gray(.075+meanLight*.075),gray(.39+meanLight*.25)};
  };
  if(chromatic<total*.012)return neutralPalette();

  auto distance=[](double a,double b){const double gap=std::abs(a-b);return std::min(gap,1.-gap);};
  const double dark=std::clamp((.42-meanLight)/.34,0.,1.);
  auto pick=[&](auto score){
   int index=-1;double best=0;Hue result;
   for(int i=0;i<hueCount;++i){
    Hue candidate;
    for(int d=-1;d<=1;++d)candidate.add(histogram[(i+d+hueCount)%hueCount]);
    if(candidate.pixels<std::max(4.,total*(.017-dark*.009)))continue;
    const double value=score(candidate);
    if(value>best){index=i;best=value;result=candidate;}
   }
   if(index>=0)for(int d=-1;d<=1;++d)histogram[(index+d+hueCount)%hueCount]={};
   return result;
  };
  const auto primary=pick([](const Hue &h){return h.weight;});
  if(!primary.pixels)return neutralPalette();
  const auto secondary=pick([&](const Hue &h){
   const double gap=distance(h.angle(),primary.angle());
   return gap<.052?0.:std::sqrt(h.weight)*std::min(1.,gap*5.);
  });
  const auto accent=pick([&](const Hue &h){
   const double gap=std::min(distance(h.angle(),primary.angle()),secondary.pixels?distance(h.angle(),secondary.angle()):.5);
   return gap<.052?0.:std::sqrt(h.weight)*std::min(1.,gap*5.);
  });
  auto tone=[](const Hue &h,double low,double high,double ceiling,double lift=0.){
   return QColor::fromHslF(h.angle(),std::min(ceiling,h.saturation/h.weight),std::clamp(h.light/h.weight+lift,low,high));
  };
  CoverPalette palette{
   tone(primary,.34,.62,.78),
   secondary.pixels?tone(secondary,.32+dark*.07,.68,.72):tone(primary,.28,.54,.60,-.08),
   tone(primary,.079,.16,.52,-.26),
   accent.pixels?tone(accent,.42+dark*.06,.70,.72):tone(primary,.42,.69,.72,.12)
  };
  if(chromatic>total*.1&&!secondary.pixels&&!accent.pixels){
   const double concentration=double(primary.pixels)/chromatic;
   palette.monochromeStrength=std::clamp((concentration-.92)/.08,0.,1.)
       *std::clamp((double(palette.primary.hslSaturationF())-.65)/.13,0.,1.);
  }
  return palette;
 }
};
