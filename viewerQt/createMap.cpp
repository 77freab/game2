#include "tile.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QMessageBox>

void skipUnknownElement(QXmlStreamReader& reader)
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

osg::Vec2i createMap(osg::ref_ptr<osg::Group> scene, 
  std::map<osg::Vec2i, blockType>& typeMap,
  std::map<osg::Vec2i, tile*>& tileMap, QString fileName)
{
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
        // читаем карту
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
        // херовый файл
      }
    }
    else
    {
      reader.readNext();
    }
  }
  file.close();

  std::map<blockType, std::string> blockTex = {
    { blockType::BORDER, "blocks/BORDER.png" },
    { blockType::BRICK, "blocks/BRICK.png" },
    { blockType::ARMOR, "blocks/ARMOR.png" },
    { blockType::WATER, "blocks/WATER.png" },
    { blockType::BUSHES, "blocks/BUSHES.png" },
    { blockType::ICE, "blocks/ICE.png" } };

  //for (int z = 3; z < 25; z++)
  //{
  //  if (z == 10)
  //    z = 16;
  //  typeMap[{4, z}] = blockType::BRICK;
  //  typeMap[{5, z}] = blockType::BRICK;

  //  typeMap[{8, z}] = blockType::BRICK;
  //  typeMap[{9, z}] = blockType::BRICK;

  //  typeMap[{20, z}] = blockType::BRICK;
  //  typeMap[{21, z}] = blockType::BRICK;

  //  typeMap[{24, z}] = blockType::BRICK;
  //  typeMap[{25, z}] = blockType::BRICK;
  //}
  //for (int z = 6; z < 25; z++)
  //{
  //  if (z == 12)
  //    z = 14;
  //  if (z == 16)
  //    z = 18;
  //  typeMap[{12, z}] = blockType::BRICK;
  //  typeMap[{13, z}] = blockType::BRICK;

  //  typeMap[{16, z}] = blockType::BRICK;
  //  typeMap[{17, z}] = blockType::BRICK;
  //}
  //for (int x = 6; x < 24; x++)
  //{
  //  if (x == 10)
  //    x = 20;
  //  typeMap[{x, 12}] = blockType::BRICK;
  //  typeMap[{x, 13}] = blockType::BRICK;
  //}
  //typeMap[{2, 13}] = blockType::BRICK;
  //typeMap[{3, 13}] = blockType::BRICK;
  //typeMap[{2, 12}] = blockType::ARMOR;
  //typeMap[{3, 12}] = blockType::ARMOR;

  //typeMap[{26, 13}] = blockType::BRICK;
  //typeMap[{27, 13}] = blockType::BRICK;
  //typeMap[{26, 12}] = blockType::ARMOR;
  //typeMap[{27, 12}] = blockType::ARMOR;

  //typeMap[{14, 19}] = blockType::ARMOR;
  //typeMap[{14, 20}] = blockType::ARMOR;
  //typeMap[{15, 19}] = blockType::ARMOR;
  //typeMap[{15, 20}] = blockType::ARMOR;

  //typeMap[{14, 9}] = blockType::BRICK;
  //typeMap[{14, 10}] = blockType::BRICK;
  //typeMap[{15, 9}] = blockType::BRICK;
  //typeMap[{15, 10}] = blockType::BRICK;

  //for (int z = 1; z < 4; z++)
  //{
  //  typeMap[{13, z}] = blockType::BRICK;
  //  typeMap[{16, z}] = blockType::BRICK;
  //}
  //typeMap[{14, 3}] = blockType::BRICK;
  //typeMap[{15, 3}] = blockType::BRICK;
  //
  //// border
  //for (int x = 2; x < 28; x++)
  //{
  //  typeMap[{x, 0}] = blockType::BORDER; // низ
  //  typeMap[{x, 27}] = blockType::BORDER; // верх
  //}
  //for (int z = 0; z < 28; z++)
  //{
  //  typeMap[{0, z}] = blockType::BORDER; // слева
  //  typeMap[{1, z}] = blockType::BORDER;

  //  for (int x = 28; x < 32; x++)
  //    typeMap[{x, z}] = blockType::BORDER; // справа
  //}

  std::map<osg::Vec2i, blockType>::const_iterator a;
  for (int x = 0; x < mapSize[0]; x++)
    for (int z = 0; z < mapSize[1]; z++)
      if ((a = typeMap.find({ x, z })) != typeMap.end())
      {
        tileMap[{ x, z }] = new tile(x * 8, 0, z * 8, blockTex[typeMap[{x, z}]]);
        scene->addChild(tileMap[{ x, z }]);
      }
  return mapSize;
}