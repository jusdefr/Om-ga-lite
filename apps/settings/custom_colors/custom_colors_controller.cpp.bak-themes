#include "custom_colors_controller.h"

#include <assert.h>

#include "palette_apply.h"

namespace OmegaLite {
namespace Colors {

namespace {

constexpr const char * k_resetText = "Restaurer les couleurs d'origine";
constexpr const char * k_saveText = "Enregistrer";
constexpr const char * k_cancelText = "Annuler";
constexpr const char * k_saveFailedText = "Echec de l'enregistrement";

} // namespace

CustomColorsController::CustomColorsController(Responder * parentResponder, ColorSettings * settings) :
    ViewController(parentResponder),
    m_settings(settings),
    m_selectableTableView(this),
    m_editorController(this, settings),
    m_saveFailed(false) {
  assert(m_settings != nullptr);
}

void CustomColorsController::didBecomeFirstResponder() {
  if (selectedRow() < 0) {
    selectCellAtLocation(0, 0);
  }
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

void CustomColorsController::viewWillAppear() {
  ViewController::viewWillAppear();
  /* The page is the transaction: nothing is written until Save. */
  m_settings->beginEdit();
  refreshRoleCells();
  refreshActionCells();
}

KDCoordinate CustomColorsController::rowHeight(int j) {
  return j < k_numberOfRoles ? ColorRoleCell::k_rowHeight : ActionCell::k_rowHeight;
}

KDCoordinate CustomColorsController::cumulatedHeightFromIndex(int j) {
  KDCoordinate height = 0;
  for (int row = 0; row < j; row++) {
    height += rowHeight(row);
  }
  return height;
}

int CustomColorsController::indexFromCumulatedHeight(KDCoordinate offsetY) {
  KDCoordinate height = 0;
  for (int row = 0; row < numberOfRows(); row++) {
    const KDCoordinate rowH = rowHeight(row);
    if (offsetY < height + rowH) {
      return row;
    }
    height += rowH;
  }
  return numberOfRows() - 1;
}

HighlightCell * CustomColorsController::reusableCell(int index, int type) {
  if (type == k_typeRole) {
    assert(index >= 0 && index < k_numberOfRoles);
    return &m_roleCells[index];
  }
  assert(type == k_typeAction);
  assert(index >= 0 && index < k_actionCount);
  return &m_actionCells[index];
}

int CustomColorsController::reusableCellCount(int type) {
  return type == k_typeRole ? k_numberOfRoles : k_actionCount;
}

int CustomColorsController::typeAtLocation(int i, int j) {
  return j < k_numberOfRoles ? k_typeRole : k_typeAction;
}

void CustomColorsController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index < k_numberOfRoles) {
    const Role role = static_cast<Role>(index);
    char hex[k_hexColorTextSize];
    writeHex(m_settings->scheme()->color(role), hex, sizeof(hex));
    static_cast<ColorRoleCell *>(cell)->setContent(roleLabel(role), hex,
                                                   m_settings->scheme()->color(role));
    return;
  }
  ActionCell * actionCell = static_cast<ActionCell *>(cell);
  if (index == k_resetRow) {
    actionCell->setText(k_resetText);
  } else if (index == k_saveRow) {
    actionCell->setText(m_saveFailed ? k_saveFailedText : k_saveText);
  } else {
    actionCell->setText(k_cancelText);
  }
}

void CustomColorsController::refreshRoleCells() {
  for (int row = 0; row < k_numberOfRoles; row++) {
    m_selectableTableView.reloadCellAtLocation(0, row);
  }
}

void CustomColorsController::refreshActionCells() {
  for (int row = k_resetRow; row < numberOfRows(); row++) {
    m_selectableTableView.reloadCellAtLocation(0, row);
  }
}

bool CustomColorsController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    const int row = selectedRow();
    if (row >= 0 && row < k_numberOfRoles) {
      m_editorController.setRole(static_cast<Role>(row));
      stackController()->push(&m_editorController);
      return true;
    }
    if (row == k_resetRow) {
      /* Restore is part of the edit: it must be possible to change your mind
       * with Cancel, so nothing is written to storage here. */
      m_settings->resetToThemeDefaults();
      m_saveFailed = false;
      refreshRoleCells();
      refreshActionCells();
      return true;
    }
    if (row == k_saveRow) {
      m_saveFailed = !m_settings->save();
      if (!m_saveFailed) {
        /* The whole interface now uses the committed palette. */
        applySchemeToPalette(*m_settings->scheme());
        stackController()->pop();
      } else {
        refreshActionCells();
      }
      return true;
    }
    if (row == k_cancelRow) {
      m_settings->revertEdit();
      applySchemeToPalette(*m_settings->scheme());
      refreshRoleCells();
      stackController()->pop();
      return true;
    }
  }
  if (event == Ion::Events::Back) {
    /* Leaving the page without saving discards the draft. Returning false lets
     * the stack view controller pop this page. */
    m_settings->revertEdit();
    applySchemeToPalette(*m_settings->scheme());
    return false;
  }
  return false;
}

} // namespace Colors
} // namespace OmegaLite
