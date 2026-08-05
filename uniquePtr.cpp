template<typename T>
class uniquePtr{

    uniquePtr(T *newT):res(newT){}
    //if u send the reference directly, res = new int(5),uniquePtr(res) &newT would give address of stack variable

    //move constructor
    uniquePtr(uniquePtr<T>&& rVal){
        res = rVal.res;
        rVal.res = nullptr;
    }
    //delete assignment
    uniquePtr& operator= (const uniquePtr<T>& rVal)=delete;

private:
    T* res;
};