#include <Geode/Geode.hpp>
#include <Geode/modify/SliderTouchLogic.hpp>
#include <Geode/modify/GJScaleControl.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "../../core/gui.hpp"

GUI_HACK_CREATE("Bypass", "Slider Limit", "Removes the limit on sliding beyond set limits", false);

class $modify(SliderLimitSliderTouchLogic, SliderTouchLogic) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");        
        hack.addHookPtr(self.getHook("SliderTouchLogic::ccTouchMoved").unwrap());
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        const auto touchPos = this->convertTouchToNodeSpace(touch);
        const auto offset = touchPos - this->m_position;

        if (this->m_rotated) {
            this->m_thumb->setPosition({0.f, offset.y});
        } else {
            this->m_thumb->setPosition({offset.x, 0.f});
        }

        if (this->m_activateThumb) this->m_thumb->activate();
        if (this->m_slider) this->m_slider->updateBar();
    }
};

class $modify(SliderLimitGJScaleControl, GJScaleControl) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");        
        hack.addHookPtr(self.getHook("GJScaleControl::ccTouchMoved").unwrap());
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (touch->m_nId != m_touchID) return;

        auto* slider = m_sliderXY;
        if (m_scaleButtonType == ScaleButtonType::X) {
            slider = m_sliderX;
        } else if (m_scaleButtonType == ScaleButtonType::Y) {
            slider = m_sliderY;
        }

        if (slider && slider->m_touchLogic->m_activateThumb) {
            const float localX = this->convertToNodeSpace(touch->getLocation()).x;
            slider->getThumb()->setPositionX(localX);
            slider->updateBar();

            const float thumbVal = slider->getThumb()->getValue();
            const float rawScale = m_lowerBound + thumbVal * (m_upperBound - m_lowerBound);
            const float baseScale = std::round(rawScale * 100.f) / 100.f;

            float valX = baseScale;
            float valY = baseScale;

            if (m_scaleButtonType == ScaleButtonType::XY && m_valueX != m_valueY) {
                if (m_valueX < m_valueY) {
                    valX = baseScale * (m_valueX / m_valueY);
                } else if (m_valueX > 0.f) {
                    valY = baseScale * (m_valueY / m_valueX);
                }
            }

            m_changedValueX = valX;
            m_changedValueY = valY;

            if (m_scaleButtonType == ScaleButtonType::X) {
                this->updateLabelX(baseScale);
                if (m_delegate) m_delegate->scaleXChanged(valX, m_scaleLocked);
            } 
            else if (m_scaleButtonType == ScaleButtonType::Y) {
                this->updateLabelY(baseScale);
                if (m_delegate) m_delegate->scaleYChanged(valY, m_scaleLocked);
            } 
            else {
                this->updateLabelXY(baseScale);
                if (m_delegate) m_delegate->scaleXYChanged(valX, valY, m_scaleLocked);
            }

            this->sliderChanged(slider->getThumb());
        }
    }
};

class $modify(SliderLimitEditorUI, EditorUI) {
    static void onModify(auto& self) {
        auto& gui = GDH::Gui::get();
        auto& hack = gui.getWindow("Bypass").findHackByName("Slider Limit");        
        hack.addHookPtr(self.getHook("EditorUI::scaleObjects").unwrap());
    }

    void scaleObjects(cocos2d::CCArray* objects, float scaleX, float scaleY, cocos2d::CCPoint pivotPoint, ObjectScaleType type, bool lockMove) {
        if (pivotPoint.x == 0.f && pivotPoint.y == 0.f) {
            pivotPoint = this->getGroupCenter(objects, false);
        }

        for (auto* obj : geode::cocos::CCArrayExt<GameObject*>(objects)) {
            if (!obj) continue;

            const auto& state = m_objectEditorStates[obj->m_uniqueID];
            
            const float newX = std::max(state.m_scaleX * scaleX * obj->m_pixelScaleX, 0.01f);
            const float newY = std::max(state.m_scaleY * scaleY * obj->m_pixelScaleY, 0.01f);

            const auto currentPos = obj->getPosition();
            const auto deltaPos = currentPos - pivotPoint;
            
            const float baseScaleX = (obj->m_scaleX != 0.f) ? obj->m_scaleX : 1.f;
            const float baseScaleY = (obj->m_scaleY != 0.f) ? obj->m_scaleY : 1.f;

            const cocos2d::CCPoint newDelta = {
                (deltaPos.x / baseScaleX) * newX,
                (deltaPos.y / baseScaleY) * newY
            };

            cocos2d::CCPoint moveVector = {0.f, 0.f};

            if (type != ObjectScaleType::Y) {
                obj->updateCustomScaleX(newX);
                moveVector.x = newDelta.x - deltaPos.x;
            }
            
            if (type != ObjectScaleType::X) {
                obj->updateCustomScaleY(newY);
                moveVector.y = newDelta.y - deltaPos.y;
            }

            if (!lockMove) {
                this->moveObject(obj, moveVector);
            }
        }
    }
};