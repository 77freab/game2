#include "tile.h"
#include <QFile>
#include <osgDB/ReadFile>

osg::ref_ptr<osg::Geode> tile::makeNewTile(blockType bt, bool pr)
{
  // ����
  osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;

  color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
  osg::ref_ptr<osg::Vec2Array> texCoord = new osg::Vec2Array;

  if (pr)
  {
    // �������
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    vertices->push_back(osg::Vec3(0, 0, 0)); // 1
    vertices->push_back(osg::Vec3(8, 0, 0)); // 2
    vertices->push_back(osg::Vec3(8, 0, 8)); // 3
    vertices->push_back(osg::Vec3(0, 0, 8)); // 4

    // �������
    normals->setBinding(osg::Array::BIND_OVERALL);
    normals->push_back(osg::Vec3(0, -1, 0));

    // ���������� ��������
    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));
  }
  else
  {
    // �������
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 4, 4));
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 8, 4));
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 12, 4));
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 16, 4));

    vertices->push_back(osg::Vec3(0,  0, 0)); // 1
    vertices->push_back(osg::Vec3(8,  0, 0)); // 2
    vertices->push_back(osg::Vec3(8, -8, 0)); // 3
    vertices->push_back(osg::Vec3(0, -8, 0)); // 4

    vertices->push_back(osg::Vec3(8,  0, 0)); // 5
    vertices->push_back(osg::Vec3(8,  0, 8)); // 6
    vertices->push_back(osg::Vec3(8, -8, 8)); // 7
    vertices->push_back(osg::Vec3(8, -8, 0)); // 8

    vertices->push_back(osg::Vec3(8,  0, 8)); // 9
    vertices->push_back(osg::Vec3(0,  0, 8)); // 10
    vertices->push_back(osg::Vec3(0, -8, 8)); // 11
    vertices->push_back(osg::Vec3(8, -8, 8)); // 12

    vertices->push_back(osg::Vec3(0,  0, 8)); // 13
    vertices->push_back(osg::Vec3(0,  0, 0)); // 14
    vertices->push_back(osg::Vec3(0, -8, 0)); // 15
    vertices->push_back(osg::Vec3(0, -8, 8)); // 16

    vertices->push_back(osg::Vec3(0, -8, 0)); // 17
    vertices->push_back(osg::Vec3(8, -8, 0)); // 18
    vertices->push_back(osg::Vec3(8, -8, 8)); // 19
    vertices->push_back(osg::Vec3(0, -8, 8)); // 20

    // �������
    normals->setBinding(osg::Array::BIND_PER_PRIMITIVE_SET);

    normals->push_back(osg::Vec3( 0,  0, -1)); // 1
    normals->push_back(osg::Vec3( 1,  0,  0)); // 2
    normals->push_back(osg::Vec3( 0,  0,  1)); // 3
    normals->push_back(osg::Vec3(-1,  0,  0)); // 4
    normals->push_back(osg::Vec3( 0, -1,  0)); // 5

    // ���������� ��������
    texCoord->setBinding(osg::Array::BIND_PER_PRIMITIVE_SET);

    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));

    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));

    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));

    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));

    texCoord->push_back(osg::Vec2(0, 0));
    texCoord->push_back(osg::Vec2(1, 0));
    texCoord->push_back(osg::Vec2(1, 1));
    texCoord->push_back(osg::Vec2(0, 1));
  }

  // ���������
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  geom->setColorArray(color);
  geom->setVertexArray(vertices);
  geom->setNormalArray(normals);
  geom->setTexCoordArray(0, texCoord);
  
  // ��������� ��������
  osg::ref_ptr<osg::Image> image = osgDB::readImageFile(blockTex[static_cast<int>(bt)]);
  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
  //texture->setImage(image);
  //texture->setUnRefImageDataAfterApply(true);
  texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  geode->addDrawable(geom);

  _tiles[static_cast<int>(bt)] = geode;
  return geode;
}

osg::ref_ptr<osg::MatrixTransform> tile::getTile(int x, int y, int z, blockType bt, bool pr)
{
  osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
  osg::Matrix m;
  m.makeTranslate(x, y, z);
  mt->setMatrix(m);

  if (_tiles[static_cast<int>(bt)] == nullptr)
  {
    makeNewTile(bt, pr);
    mt->addChild(_tiles[static_cast<int>(bt)]);
  }
  else
    mt->addChild(_tiles[static_cast<int>(bt)]);

  return mt;
}

tile::tile()
{
  blockTex.push_back("./Resources/blocks/BORDER.png");
  blockTex.push_back("./Resources/blocks/BRICK.png");
  blockTex.push_back("./Resources/blocks/ARMOR.png");
  blockTex.push_back("./Resources/blocks/WATER.png");
  blockTex.push_back("./Resources/blocks/BUSHES.png");
  blockTex.push_back("./Resources/blocks/ICE.png");

  blockTex.push_back("./Resources/projectile/UP.png");
  blockTex.push_back("./Resources/projectile/DOWN.png");
  blockTex.push_back("./Resources/projectile/LEFT.png");
  blockTex.push_back("./Resources/projectile/RIGHT.png");
}

void tile::skipUnknownElement(QXmlStreamReader& reader)
{
  reader.readNext();
  while (!reader.atEnd())
  {
    if (reader.isEndElement())
    {
      QString aa = reader.name().toString();
      bool a = false;
      reader.readNext();
      break;
    }
    if (reader.isStartElement())
    {
      QString aa = reader.name().toString();
      bool a = false;
      if (reader.name() == "x" || reader.name() == "z")
        skipUnknownElement(reader);
      if (reader.name() == "tile")
        break;
    }
    else
    {
      reader.readNext();
    }
  }
}

osg::Vec2i tile::createMap(osg::ref_ptr<osg::Group> scene,
  std::map<osg::Vec2i, blockType>& typeMap,
  std::map<osg::Vec2i, osg::ref_ptr<osg::MatrixTransform>>& tileMap, QString fileName)
{
  _tiles.clear();
  for (int i = 0; i < 10; i++)
    _tiles.push_back(nullptr);

  osg::Vec2i mapSize;
  QXmlStreamReader reader;
  QFile file(fileName);
  file.open(QIODevice::ReadOnly | QFile::Text);
  reader.setDevice(&file);
  reader.readNext();
  while (!reader.atEnd())
  {
    if (reader.isStartElement())
    {
      QString aa = reader.name().toString();
      bool a = false;
      if (reader.name() == "map")
      {
        // ������ �����
        reader.readNext();
        while (!reader.atEnd())
        {
          if (reader.isEndElement())
          {
            QString aa = reader.name().toString();
            bool a = false;
            reader.readNext();
          }
          if (reader.isStartElement())
          {
            QString aa = reader.name().toString();
            bool a = false;
            if (reader.name() == "sizeX")
            {
              mapSize[0] = (reader.readElementText().toInt() + 48) / 8;
            }
            else if (reader.name() == "sizeZ")
            {
              mapSize[1] = (reader.readElementText().toInt() + 32) / 8;
            }
            else if (reader.name() == "tile")
            {
              blockType type;
              int x, z;
              QString attrStr = reader.attributes()[0].name().toString();
              if (attrStr == "type")
              {
                QString attrValue = reader.attributes()[0].value().toString();
                if (attrValue == "BORDER") type = blockType::BORDER;
                else if (attrValue == "BRICK") type = blockType::BRICK;
                else if (attrValue == "ARMOR") type = blockType::ARMOR;
                else if (attrValue == "WATER") type = blockType::WATER;
                else if (attrValue == "BUSHES") type = blockType::BUSHES;
                else if (attrValue == "ICE") type = blockType::ICE;
              }
              reader.readNext();
              while (!reader.atEnd())
              {
                if (reader.isEndElement())
                {
                  QString aa = reader.name().toString();
                  bool a = false;
                  if (reader.name() == "tile")
                    break;
                  reader.readNext();
                }
                if (reader.isStartElement())
                {
                  QString aa = reader.name().toString();
                  bool a = false;
                  if (reader.name() == "x")
                  {
                    x = reader.readElementText().toInt() / 8;
                  }
                  else if (reader.name() == "z")
                  {
                    z = reader.readElementText().toInt() / 8;
                  }
                  else
                  {
                    skipUnknownElement(reader);
                  }
                }
                else
                {
                  reader.readNext();
                }
              }
              typeMap[{x, z}] = type;
            }
            else
            {
              skipUnknownElement(reader);
            }
          }
          else
          {
            reader.readNext();
          }
        }
      }
      else
      {
        // ������� ����
      }
    }
    else
    {
      reader.readNext();
    }
  }
  file.close();

  std::map<osg::Vec2i, blockType>::const_iterator it;
  for (int x = 0; x < mapSize[0]; x++)
    for (int z = 0; z < mapSize[1]; z++)
      if ((it = typeMap.find({ x, z })) != typeMap.end())
      {
        if ((*it).second == blockType::WATER || (*it).second == blockType::ICE)
          tileMap[{ x, z }] = getTile(x * 8, 0, z * 8, (*it).second, true);
        else
          tileMap[{ x, z }] = getTile(x * 8, 0, z * 8, (*it).second);
        scene->addChild(tileMap[{ x, z }]);
      }
  return mapSize;
}