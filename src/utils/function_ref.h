#ifndef __UTILS_FUNCTION_REF__
#define __UTILS_FUNCTION_REF__

namespace utils {

    template<typename Fun>
    class function_ref;
    
    template<typename RetType, typename ... Args>
    class function_ref<RetType(Args...)> {
    private:
        const void *data;
        RetType (*thunk)(const void *, Args ...);
    
    public:
        template<typename Function>
        function_ref(const Function &fn)
            : data{&fn}
            , thunk{[](const void *data, Args ... args) {
                return std::invoke(*static_cast<const Function *>(data), std::forward<Args>(args) ...);
            }} {}
    
        RetType operator()(Args ... args) const {
            return (*thunk)(data, std::forward<Args>(args) ...);
        }
    };
}

#endif