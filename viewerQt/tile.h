#pragma once

#include <string>
#include <osg/MatrixTransform>

#include <osg/Texture2D>

class tile : public osg::MatrixTransform
{
public:
  tile(int x, int y, int z, std::string texPath, bool pr = false);
  void setTexture(std::string texPath);
private:
  osg::ref_ptr<osg::Geode> _geode;
  osg::ref_ptr<osg::Geometry> _geom;
  osg::ref_ptr<osg::Vec4Array> _color;
  osg::ref_ptr<osg::Vec3Array> _vertices;
  osg::ref_ptr<osg::Vec3Array> _normals;
  osg::ref_ptr<osg::Vec2Array> _texCoord;
  osg::ref_ptr<osg::Image> _image;
  osg::ref_ptr<osg::Texture2D> _texture;
};