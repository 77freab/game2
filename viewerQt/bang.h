#pragma once

#include <chrono>
#include <osg/MatrixTransform>
#include <osg/Texture2D>

class Bang : public osg::MatrixTransform
{
public:
  Bang(int x, int y, int z, std::list<osg::Node*>& toDelete);
  Bang( const Bang&);
  Bang& operator=(const Bang&) = delete;
  
protected:
  virtual ~Bang() {}

private:
  double _timer;
  osg::ref_ptr<osg::Geometry> _geom;
  std::list<osg::Node*>& _toDelete;
  int _animateStage;

  //! called from update callback to animate the explosion
  void AnimateBang(double simTime);

  class BangCallback;
};