#include "bang.h"
#include <osgDB/ReadFile>

void bangCallback::operator()(osg::Node* nd, osg::NodeVisitor* ndv)
{
  bang* bng = dynamic_cast<bang*>(nd);
  bng->makeBang();
  traverse(nd, ndv);
}

bang::bang(int x, int y, int z, std::list<osg::Node*>* toDelete)
  : _geode(new osg::Geode), _normals(new osg::Vec3Array), _geom(new osg::Geometry),
  _vertices(new osg::Vec3Array), _color(new osg::Vec4Array), _texCoord(new osg::Vec2Array),
  _texture(new osg::Texture2D), _image(osgDB::readImageFile("./Resources/bang.png")),
  _toDelete(toDelete), _clb(new bangCallback)
{
  this->setDataVariance(osg::Object::DYNAMIC);
  this->setUpdateCallback(_clb);

  osg::Matrix m;
  m.makeTranslate(x, y, z);
  this->setMatrix(m);

  _geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  _color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
  _geom->setColorArray(_color.get());

  _vertices->push_back(osg::Vec3(-8, 0, -8)); // 1
  _vertices->push_back(osg::Vec3( 8, 0, -8)); // 2
  _vertices->push_back(osg::Vec3( 8, 0,  8)); // 3
  _vertices->push_back(osg::Vec3(-8, 0,  8)); // 4

  _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

  _geom->setVertexArray(_vertices.get());

  _normals->setBinding(osg::Array::BIND_OVERALL);
  _normals->push_back(osg::Vec3(0, -1, 0));

  _geom->setNormalArray(_normals.get());

  _texCoord->push_back(osg::Vec2(2. / 3, 0));
  _texCoord->push_back(osg::Vec2(1, 0));
  _texCoord->push_back(osg::Vec2(1, 1));
  _texCoord->push_back(osg::Vec2(2. / 3, 1));

  _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);

  _texture->setImage(_image);
  _texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  _texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = _geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, _texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  _geode->addDrawable(_geom.get());
  this->addChild(_geode.get());
}

void bang::makeBang()
{
  int temp = 10;
  _roughTimer++;
  if (_roughTimer == temp)
  {
    _texCoord = new osg::Vec2Array;

    _texCoord->push_back(osg::Vec2(1. / 3, 0));
    _texCoord->push_back(osg::Vec2(2. / 3, 0));
    _texCoord->push_back(osg::Vec2(2. / 3, 1));
    _texCoord->push_back(osg::Vec2(1. / 3, 1));

    _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
  }
  if (_roughTimer == temp * 2)
  {
    _texCoord = new osg::Vec2Array;

    _texCoord->push_back(osg::Vec2(0, 0));
    _texCoord->push_back(osg::Vec2(1. / 3, 0));
    _texCoord->push_back(osg::Vec2(1. / 3, 1));
    _texCoord->push_back(osg::Vec2(0, 1));

    _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
  }
  if (_roughTimer == temp * 3)
  {
    _toDelete->push_back(this);
    this->removeUpdateCallback(_clb);
  }
}