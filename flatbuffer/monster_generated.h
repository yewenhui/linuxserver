// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_
#define FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_

#include "flatbuffers/flatbuffers.h"


namespace MyGame {
namespace Sample {

struct Vec3;
struct Monster;

enum {
  Color_Red = 0,
  Color_Green = 1,
  Color_Blue = 2
};

inline const char **EnumNamesColor() {
  static const char *names[] = { "Red", "Green", "Blue", nullptr };
  return names;
}

inline const char *EnumNameColor(int e) { return EnumNamesColor()[e]; }

enum {
  Any_NONE = 0,
  Any_Monster = 1
};

inline const char **EnumNamesAny() {
  static const char *names[] = { "NONE", "Monster", nullptr };
  return names;
}

inline const char *EnumNameAny(int e) { return EnumNamesAny()[e]; }

bool VerifyAny(const flatbuffers::Verifier &verifier, const void *union_obj, uint8_t type);

MANUALLY_ALIGNED_STRUCT(4) Vec3 {
 private:
  float x_;
  float y_;
  float z_;

 public:
  Vec3(float x, float y, float z)
    : x_(flatbuffers::EndianScalar(x)), y_(flatbuffers::EndianScalar(y)), z_(flatbuffers::EndianScalar(z)) { }

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  float z() const { return flatbuffers::EndianScalar(z_); }
};
STRUCT_END(Vec3, 12);

struct Monster : private flatbuffers::Table {
  const Vec3 *pos() const { return GetStruct<const Vec3 *>(4); }
  int16_t mana() const { return GetField<int16_t>(6, 150); }
  int16_t hp() const { return GetField<int16_t>(8, 100); }
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(10); }
  const flatbuffers::Vector<uint8_t> *inventory() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(14); }
  int8_t color() const { return GetField<int8_t>(16, 2); }
  bool Verify(const flatbuffers::Verifier &verifier) const {
    return VerifyTable(verifier) &&
           VerifyField<Vec3>(verifier, 4 /* pos */) &&
           VerifyField<int16_t>(verifier, 6 /* mana */) &&
           VerifyField<int16_t>(verifier, 8 /* hp */) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 10 /* name */) &&
           verifier.Verify(name()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 14 /* inventory */) &&
           verifier.Verify(inventory()) &&
           VerifyField<int8_t>(verifier, 16 /* color */);
  }
};

struct MonsterBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_pos(const Vec3 *pos) { fbb_.AddStruct(4, pos); }
  void add_mana(int16_t mana) { fbb_.AddElement<int16_t>(6, mana, 150); }
  void add_hp(int16_t hp) { fbb_.AddElement<int16_t>(8, hp, 100); }
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(10, name); }
  void add_inventory(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory) { fbb_.AddOffset(14, inventory); }
  void add_color(int8_t color) { fbb_.AddElement<int8_t>(16, color, 2); }
  MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  MonsterBuilder &operator=(const MonsterBuilder &);
  flatbuffers::Offset<Monster> Finish() { return flatbuffers::Offset<Monster>(fbb_.EndTable(start_, 7)); }
};

inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb,
   const Vec3 *pos = 0,
   int16_t mana = 150,
   int16_t hp = 100,
   flatbuffers::Offset<flatbuffers::String> name = 0,
   flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory = 0,
   int8_t color = 2) {
  MonsterBuilder builder_(_fbb);
  builder_.add_inventory(inventory);
  builder_.add_name(name);
  builder_.add_pos(pos);
  builder_.add_hp(hp);
  builder_.add_mana(mana);
  builder_.add_color(color);
  return builder_.Finish();
}

bool VerifyAny(const flatbuffers::Verifier &verifier, const void *union_obj, uint8_t type) {
  switch (type) {
    case Any_NONE: return true;
    case Any_Monster: return verifier.VerifyTable(reinterpret_cast<const Monster *>(union_obj));
    default: return false;
  }
}

inline const Monster *GetMonster(const void *buf) { return flatbuffers::GetRoot<Monster>(buf); }

inline bool VerifyMonsterBuffer(const flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<Monster>(); }

inline void FinishMonsterBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<Monster> root) { fbb.Finish(root); }

}  // namespace Sample
}  // namespace MyGame

#endif  // FLATBUFFERS_GENERATED_MONSTER_MYGAME_SAMPLE_H_
