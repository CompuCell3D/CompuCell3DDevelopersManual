#include <cc3d/kernel.h>

#include <iostream>

namespace {

class MinimalClassSteppable {
public:
    void init(CC3DKernelContext *) {}

    void start(CC3DKernelContext *) {
        stepCount_ = 0;
        std::cout << "class steppable start" << std::endl;
    }

    void step(CC3DKernelContext *ctx) {
        ++stepCount_;

        if (ctx->mcs % 10 == 0) {
            std::cout << "class steppable mcs=" << ctx->mcs
                      << " steps=" << stepCount_ << std::endl;
        }
    }

    void finish(CC3DKernelContext *) {
        std::cout << "class steppable finish steps="
                  << stepCount_ << std::endl;
    }

private:
    unsigned long long stepCount_ = 0;
};

void *create() {
    return new MinimalClassSteppable();
}

void init(CC3DKernelContext *ctx, void *state) {
    static_cast<MinimalClassSteppable *>(state)->init(ctx);
}

void start(CC3DKernelContext *ctx, void *state) {
    static_cast<MinimalClassSteppable *>(state)->start(ctx);
}

void step(CC3DKernelContext *ctx, void *state) {
    static_cast<MinimalClassSteppable *>(state)->step(ctx);
}

void finish(CC3DKernelContext *ctx, void *state) {
    static_cast<MinimalClassSteppable *>(state)->finish(ctx);
}

void destroy(void *state) {
    delete static_cast<MinimalClassSteppable *>(state);
}

const CC3DSteppableV1 api = {
    CC3D_KERNEL_ABI_VERSION,
    &create,
    &start,
    &step,
    &finish,
    &destroy,
    &init,
};

} // namespace

extern "C" CC3D_STEPPABLE_EXPORT
const CC3DSteppableV1 *cc3d_get_steppable_v1() {
    return &api;
}
