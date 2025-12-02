class AC {
public:
    virtual void fkt1() = 0;
    virtual void fkt2() = 0;
    void init() {
        fkt1();
        fkt2();
    }
    virtual ~AC() = default;z
};

class Derived : public AC {
public:
    Derived() {
        init(); // safe here
    }
    void fkt1() override { /*...*/ }
    void fkt2() override { /*...*/ }
};
