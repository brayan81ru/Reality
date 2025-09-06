#pragma once


namespace Reality {
    class BaseComponent abstract {
        private:
        public:
            BaseComponent() = default;
            virtual ~BaseComponent();
            virtual void Update();
            virtual void FixedUpdate();
            virtual void LateUpdate();
            virtual void Awake();
            virtual void Start();
    };
}
