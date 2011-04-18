#ifndef __itkDiceOverlapImageFilter_h
#define __itkDiceOverlapImageFilter_h

#include "itkImageToImageFilter.h"
#include <map>
#include <set>

template <typename T>
std::ostream& operator<<(std::ostream& output, const std::set<T> &setobject);
 
namespace itk
{

/** \class DiceOverlapImageFilter
 * \brief Computes the Dice overlap per label
 *
 * \ingroup IntensityImageFilters
 * \ingroup Multithreaded
 */

template < typename TInputImage >
class ITK_EXPORT DiceOverlapImageFilter:
    public ImageToImageFilter< TInputImage, TInputImage >
{
public:

  /** Standard class typedefs. */
  typedef DiceOverlapImageFilter                          Self;
  typedef ImageToImageFilter< TInputImage, TInputImage >  Superclass;
  typedef SmartPointer<Self>                              Pointer;
  typedef SmartPointer<const Self>                        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( DiceOverlapImageFilter, ImageToImageFilter );

  /** Image dimension. */
  itkStaticConstMacro( ImageDimension, unsigned int, TInputImage::ImageDimension);

  /** Pixel Type of the input image */
  typedef TInputImage                                       InputImageType;
  typedef typename InputImageType::Pointer                  InputImagePointer;
  typedef typename InputImageType::ConstPointer             InputImageConstPointer;
  typedef typename InputImageType::PixelType                InputPixelType;
  typedef typename InputImageType::RegionType               InputImageRegionType;
  typedef typename NumericTraits<
    InputPixelType>::RealType                               RealType;
  typedef typename NumericTraits<
    InputPixelType>::ScalarRealType                         ScalarRealType;

  /** Dice computation related typedefs. */
  typedef std::map<InputPixelType, std::size_t>             OverlapMapType;
  typedef std::map<InputPixelType, ScalarRealType>          OverlapMapRealType;
  typedef std::set<InputPixelType>                          LabelsType;

  /** Set and get the user-requested labels for which the overlaps a. */
  itkSetMacro( RequestedLabels, LabelsType );
  itkGetConstReferenceMacro( RequestedLabels, LabelsType );

  /** Get the Dice overlaps, all of them. */
  itkGetConstReferenceMacro( DiceOverlap, OverlapMapRealType );

  /** Print the Dice overlaps, only the requested ones. */
  void PrintRequestedDiceOverlaps( void );

// #ifdef ITK_USE_CONCEPT_CHECKING
//   /** Begin concept checking */
//   itkConceptMacro( InputHasNumericTraitsCheck,
//     (Concept::HasNumericTraits<InputPixelType>) );
//   /** End concept checking */
// #endif

protected:
  DiceOverlapImageFilter();
  virtual ~DiceOverlapImageFilter() {};

  /** PrintSelf. */
  void PrintSelf( std::ostream& os, Indent indent ) const;

  virtual void BeforeThreadedGenerateData( void );
  virtual void AfterThreadedGenerateData( void );

  /** ThreadedGenerateData. */
  virtual void ThreadedGenerateData(
    const InputImageRegionType & outputRegionForThread,
    int threadId );

private:
  DiceOverlapImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Member variables. */
  LabelsType                    m_RequestedLabels;

  std::vector<OverlapMapType>   m_SumA;
  std::vector<OverlapMapType>   m_SumB;
  std::vector<OverlapMapType>   m_SumC;
  OverlapMapRealType            m_DiceOverlap;

}; // end class DiceOverlapImageFilter

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiceOverlapImageFilter.txx"
#endif

#endif // end #ifndef __itkDiceOverlapImageFilter_h
