#include <cstring>
#include <string>

#include "test_framework.h"

#include "color_roles.h"

using namespace OmegaLite::Colors;

TEST(RoleCountIsTheAdvertisedSixteen) {
  CHECK_EQ(k_numberOfRoles, 16);
  CHECK_EQ(static_cast<int>(Role::NumberOfRoles), 16);
  CHECK_EQ(roleIndex(Role::Background), 0);
  CHECK_EQ(roleIndex(Role::GraphAxis), 15);
}

TEST(RoleValidityBounds) {
  for (uint8_t index = 0; index < k_numberOfRoles; index++) {
    CHECK(isValidRole(index));
  }
  CHECK(!isValidRole(k_numberOfRoles));
  CHECK(!isValidRole(42));
}

TEST(RoleIdentifiersAreUniqueAndNotEmpty) {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    const char * identifier = roleIdentifier(static_cast<Role>(i));
    CHECK(identifier != nullptr && identifier[0] != '\0');
    for (uint8_t j = i + 1; j < k_numberOfRoles; j++) {
      CHECK(std::strcmp(identifier, roleIdentifier(static_cast<Role>(j))) != 0);
    }
  }
}

TEST(RoleLabelsAndPaletteKeysAreNotEmpty) {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    const Role role = static_cast<Role>(i);
    CHECK(roleLabel(role)[0] != '\0');
    CHECK(paletteKey(role)[0] != '\0');
  }
}

TEST(PaletteKeysAreUnique) {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    for (uint8_t j = i + 1; j < k_numberOfRoles; j++) {
      CHECK(std::strcmp(paletteKey(static_cast<Role>(i)),
                        paletteKey(static_cast<Role>(j))) != 0);
    }
  }
}
