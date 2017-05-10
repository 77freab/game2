#pragma once

#include <osg/MatrixTransform>
#include <osg/Texture2D>

class bangCallback;

class bang : public osg::MatrixTransform
{
public:
  bang(int x, int y, int z, std::list<osg::Node*>& toDelete);
  void AnimateBang();
private:
  osg::ref_ptr<osg::Geode> _geode;
  osg::ref_ptr<osg::Geometry> _geom;
  osg::ref_ptr<osg::Vec4Array> _color;
  osg::ref_ptr<osg::Vec3Array> _vertices;
  osg::ref_ptr<osg::Vec3Array> _normals;
  osg::ref_ptr<osg::Vec2Array> _texCoord;
  osg::ref_ptr<osg::Image> _image;
  osg::ref_ptr<osg::Texture2D> _texture;
  int _roughTimer = 0;
  std::list<osg::Node*>* _toDelete;
  osg::ref_ptr<bangCallback> _clb;
};