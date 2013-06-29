#ifndef INTERPOLATIONBUFFER_H
#define INTERPOLATIONBUFFER_H


///////////////////////////////////////////////////////////////////////////////////////////////
/// Templated interpolation buffer. Used to smoothly interpolate between stored elements. The
/// interplation is delimited by the time value.
/// ScalarType: the type used for the arithmetic (float or double)
/// ElementType: The type of the element in the interpolation buffer. Needs to provide:
///     ElementType& operator *(const ScalarType rhs) : result of operation with a scalar
///     ElementType& operator +(const ElementType& rhs) : result of addition with an element
template< typename ElementType, typename ScalarType >
struct InterpolationBuffer
{
    std::vector<ElementType> Elements;
    ScalarType StartTime;
    ScalarType EndTime;
    ScalarType AverageDt;
    InterpolationBuffer(unsigned int uSize = 1000)
    {
        Elements.reserve(uSize);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief AddElement - Adds an element to the interpolation buffer, updates the average and end
    /// times
    /// \param element - the new element to add
    ///
    void AddElement(const ElementType& element)
    {
        assert(element.Time > EndTime);
        const size_t nElems = Elements.size();
        const ScalarType dt = nElems == 0 ? 0 : element.Time - Elements.back().Time;
        // update the average dt
        AverageDt = (AverageDt*nElems + dt)/(nElems+1);
        // add the element and update the end time
        Elements.push_back(element);
        EndTime = element.Time;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief GetNext - Gets the next element in the buffer, depending on the maximum time interval
    /// specified
    /// \param dMaxTime - The maximum time interval. The returned element time will be <= to this
    /// \param indexOut - The output index of the returned element
    /// \param output - The output element
    /// \return - True if the function returned an intermediate element, false if we had to interpolate
    /// in which case we've reached the end
    ///
    bool GetNext(const ScalarType dMaxTime, size_t& indexOut, ElementType& output)
    {
        // if we have reached the end, interpolate and signal the end
        if(indexOut + 1 >= Elements.size()){
            output = GetElement(dMaxTime,&indexOut);
            return false;
        }else if( Elements[indexOut+1] < dMaxTime ){
            output = GetElement(dMaxTime,&indexOut);
            return false;
        }else{
            indexOut++;
            output = Elements[indexOut];
            return true;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief GetElement - Returns an interpolated element
    /// \param dTime - The time for which we require the element
    /// \return - The element
    ///
    ElementType GetElement(const ScalarType dTime)
    {
        size_t index;
        return GetElement(dTime,&index);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief GetElement - Returns an interpolated element
    /// \param dTime - The time for which we require the element
    /// \param pIndex - The output index
    /// \return - The element
    ///
    ElementType GetElement(const ScalarType dTime, size_t* pIndex) {
        assert(dTime >= StartTime && dTime <= EndTime);
        // guess what the index would be
        size_t guessIdx = (dTime-StartTime)/AverageDt;
        const size_t nElements = Elements.size();
        // now using the guess, find a direction to go
        if( Elements[guessIdx].Time > dTime ){
            // we need to go backwards
            if(guessIdx == 0){
                *pIndex = guessIdx;
                return Elements.front();
            }

            while((guessIdx-1) > 0 && Elements[guessIdx-1].Time > dTime)
            {
                guessIdx--;
            }
            const ScalarType interpolator = (dTime - Elements[guessIdx-1].Time)/(Elements[guessIdx].Time-Elements[guessIdx-1].Time);
            *pIndex = guessIdx-1;
            return Elements[guessIdx-1]*(1-interpolator) + Elements[guessIdx]*interpolator;
        }else{
            // we need to go forwards
            if(guessIdx == nElements-1){
                *pIndex = guessIdx;
                return Elements.back();
            }

            while((guessIdx+1) < nElements && Elements[guessIdx+1].Time < dTime)
            {
                guessIdx++;
            }
            const ScalarType interpolator = (dTime - Elements[guessIdx].Time)/(Elements[guessIdx+1].Time-Elements[guessIdx].Time);
            *pIndex = guessIdx;
            return Elements[guessIdx]*(1-interpolator) + Elements[guessIdx+1]*interpolator;
        }
    }
};


#endif // INTERPOLATIONBUFFER_H