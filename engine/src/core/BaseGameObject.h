#pragma once
#include <string>
#include <vector>

#include "BaseComponent.h"


namespace Reality {
    class BaseGameObject abstract{
        public:
        BaseGameObject();
        virtual ~BaseGameObject() = default;

        virtual void Destroy();

        void SetName(const std::string &_name){  m_Name = _name;}

        std::string GetName(){ return m_Name; }

        void AddComponent(const BaseComponent* _component);

        private:

        std::vector<BaseComponent> m_components;

        int id = 0;

        std::string m_Name;
    };
}
