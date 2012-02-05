/*=========================================================================
*
* Copyright Marius Staring, Stefan Klein, David Doria. 2011.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0.txt
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*=========================================================================*/
/** \file
 \brief This program computes texture features based on the gray-level co-occurrence matrix (GLCM).
 
 \verbinclude texture.help
 */
#include "itkCommandLineArgumentParser.h"
#include "ITKToolsHelpers.h"
#include "ITKToolsBase.h"

#include <itksys/SystemTools.hxx>

#include "itkTextureImageToImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMultiThreader.h"


/**
 * ******************* GetHelpString *******************
 */

std::string GetHelpString( void )
{
  std::stringstream ss;
  ss << "ITKTools v" << itktools::GetITKToolsVersion() << "\n"
  << "Usage:" << std::endl
  << "pxtexture" << std::endl
  << "This program computes texture features based on the gray-level co-occurrence matrix (GLCM)." << std::endl
  << "  -in      inputFilename" << std::endl
  << "  [-out]   outputDirectory, default equal to the inputFilename directory" << std::endl
  << "  [-r]     the radius of the neighborhood on which to construct the GLCM, default 3" << std::endl
  << "  [-os]    the desired offset scales to compute the GLCM, default 1, but can be e.g. 1 2 4" << std::endl
  << "  [-b]     the number of bins of the GLCM, default 128" << std::endl
  << "  [-noo]   the number of texture feature outputs, default all 8" << std::endl
  << "  [-opct]  output pixel component type, default float" << std::endl
  << "Supported: 2D, 3D, any input image type, float or double output type.";

  return ss.str();

} // end GetHelpString()


// To print the progress
class ShowProgressObject
{
public:
  ShowProgressObject( itk::ProcessObject* o )
  {
    this->m_Process = o;
  }
  void ShowProgress()
  {
    std::cout << "\rProgress: "
      << static_cast<unsigned int>( 100.0 * this->m_Process->GetProgress() ) << "%";
  }
  itk::ProcessObject::Pointer m_Process;
};

//-------------------------------------------------------------------------------------


/** Texture */

class ITKToolsTextureBase : public itktools::ITKToolsBase
{ 
public:
  ITKToolsTextureBase()
  {
    this->m_InputFileName = "";
    this->m_OutputDirectory = "";
    this->m_NeighborhoodRadius = 0;
    //std::vector< unsigned int > this->m_OffsetScales;
    this->m_NumberOfBins = 0;
    this->m_NumberOfOutputs = 0;
  };
  ~ITKToolsTextureBase(){};

  /** Input parameters */
  std::string m_InputFileName;
  std::string m_OutputDirectory;
  unsigned int m_NeighborhoodRadius;
  std::vector< unsigned int > m_OffsetScales;
  unsigned int m_NumberOfBins;
  unsigned int m_NumberOfOutputs;
    
}; // end TextureBase


template< class TInputComponentType, class TOutputComponentType, unsigned int VDimension >
class ITKToolsTexture : public ITKToolsTextureBase
{
public:
  typedef ITKToolsTexture Self;

  ITKToolsTexture(){};
  ~ITKToolsTexture(){};

  static Self * New( itktools::ComponentType inputComponentType,
		     itktools::ComponentType outputComponentType, unsigned int dim )
  {
    if ( itktools::IsType<TInputComponentType>( inputComponentType ) &&
	 itktools::IsType<TOutputComponentType>( outputComponentType ) && VDimension == dim )
    {
      return new Self;
    }
    return 0;
  }

  void Run( void )
  {
    /** Typedefs. */
    typedef itk::Image<TInputComponentType, VDimension>   InputImageType;
    typedef itk::Image<TOutputComponentType, VDimension>  OutputImageType;
    typedef itk::TextureImageToImageFilter<
      InputImageType, OutputImageType >                   TextureFilterType;
    typedef itk::ImageFileReader< InputImageType >        ReaderType;
    typedef itk::ImageFileWriter< OutputImageType >       WriterType;

    /** Read the input. */
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( this->m_InputFileName.c_str() );

    /** Setup the texture filter. */
    typename TextureFilterType::Pointer textureFilter = TextureFilterType::New();
    textureFilter->SetInput( reader->GetOutput() );
    textureFilter->SetNeighborhoodRadius( this->m_NeighborhoodRadius );
    textureFilter->SetOffsetScales( this->m_OffsetScales );
    textureFilter->SetNumberOfHistogramBins( this->m_NumberOfBins );
    textureFilter->SetNormalizeHistogram( false );
    textureFilter->SetNumberOfRequestedOutputs( this->m_NumberOfOutputs );

    /** Create and attach a progress observer. */
    ShowProgressObject progressWatch( textureFilter );
    itk::SimpleMemberCommand<ShowProgressObject>::Pointer command
      = itk::SimpleMemberCommand<ShowProgressObject>::New();
    command->SetCallbackFunction( &progressWatch, &ShowProgressObject::ShowProgress );
    textureFilter->AddObserver( itk::ProgressEvent(), command );

    /** Create the output file names. */
    std::vector< std::string > outputFileNames( 8, "" );
    outputFileNames[ 0 ] = this->m_OutputDirectory + "energy.mhd";
    outputFileNames[ 1 ] = this->m_OutputDirectory + "entropy.mhd";
    outputFileNames[ 2 ] = this->m_OutputDirectory + "correlation.mhd";
    outputFileNames[ 3 ] = this->m_OutputDirectory + "inverseDifferenceMoment.mhd";
    outputFileNames[ 4 ] = this->m_OutputDirectory + "inertia.mhd";
    outputFileNames[ 5 ] = this->m_OutputDirectory + "clusterShade.mhd";
    outputFileNames[ 6 ] = this->m_OutputDirectory + "clusterProminence.mhd";
    outputFileNames[ 7 ] = this->m_OutputDirectory + "HaralickCorrelation.mhd";

    /** Setup and process the pipeline. */
    for ( unsigned int i = 0; i < this->m_NumberOfOutputs; ++i )
    {
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( outputFileNames[ i ].c_str() );
      writer->SetInput( textureFilter->GetOutput( i ) );
      writer->Update();
    }
  }

}; // end Texture

//-------------------------------------------------------------------------------------

/* Declare PerformTextureAnalysis. */
template< class InputImageType, class OutputImageType >
void PerformTextureAnalysis(
  const std::string & inputFileName,
  const std::string & outputDirectory,
  unsigned int neighborhoodRadius,
  const std::vector< unsigned int > & offsetScales,
  unsigned int numberOfBins,
  unsigned int numberOfOutputs );

//-------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
  /** Create a command line argument parser. */
  itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
  parser->SetCommandLineArguments( argc, argv );
  parser->SetProgramHelpText( GetHelpString() );

  parser->MarkArgumentAsRequired( "-in", "The input filename." );

  itk::CommandLineArgumentParser::ReturnValue validateArguments = parser->CheckForRequiredArguments();

  if( validateArguments == itk::CommandLineArgumentParser::FAILED )
  {
    return EXIT_FAILURE;
  }
  else if( validateArguments == itk::CommandLineArgumentParser::HELPREQUESTED )
  {
    return EXIT_SUCCESS;
  }
  
  /** Get arguments. */
  std::string inputFileName = "";
  parser->GetCommandLineArgument( "-in", inputFileName );

  std::string base = itksys::SystemTools::GetFilenamePath( inputFileName );
  if ( base != "" ) base = base + "/";
  std::string outputDirectory = base;
  parser->GetCommandLineArgument( "-out", outputDirectory );
  bool endslash = itksys::SystemTools::StringEndsWith( outputDirectory.c_str(), "/" );
  if ( !endslash ) outputDirectory += "/";

  unsigned int neighborhoodRadius = 3;
  parser->GetCommandLineArgument( "-r", neighborhoodRadius );

  std::vector<unsigned int> offsetScales( 1, 1 );
  parser->GetCommandLineArgument( "-os", offsetScales );

  unsigned int numberOfBins = 128;
  parser->GetCommandLineArgument( "-b", numberOfBins );

  unsigned int numberOfOutputs = 8;
  parser->GetCommandLineArgument( "-noo", numberOfOutputs );

  std::string componentTypeOutString = "float";
  parser->GetCommandLineArgument( "-opct", componentTypeOutString );

  /** Check that numberOfOutputs <= 8. */
  if ( numberOfOutputs > 8 )
  {
    std::cerr << "ERROR: The maximum number of outputs is 8. You requested "
      << numberOfOutputs << "." << std::endl;
    return 1;
  }

  /** Threads. */
  unsigned int maximumNumberOfThreads
    = itk::MultiThreader::GetGlobalDefaultNumberOfThreads();
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(
    maximumNumberOfThreads );

  /** Determine image properties. */
  std::string componentTypeIn = "short";
  std::string PixelType; //we don't use this
  unsigned int Dimension = 3;
  unsigned int NumberOfComponents = 1;
  std::vector<unsigned int> imagesize( Dimension, 0 );
  int retgip = itktools::GetImageProperties(
    inputFileName,
    PixelType,
    componentTypeIn,
    Dimension,
    NumberOfComponents,
    imagesize );
  if ( retgip != 0 )
  {
    return 1;
  }

  /** Check for vector images. */
  if ( NumberOfComponents > 1 )
  {
    std::cerr << "ERROR: The NumberOfComponents is larger than 1!" << std::endl;
    std::cerr << "Vector images are not supported." << std::endl;
    return 1;
  }

  /** Class that does the work */
  ITKToolsTextureBase * texture = NULL;

  /** Short alias */
  unsigned int dim = Dimension;
 
  /** Input images are read in as float, always. The default output is float,
   * but can be overridden by specifying -opct in the command line.
   */
  //itktools::EnumComponentType inputComponentType = itktools::GetImageComponentType( inputFileName );
  itktools::ComponentType inputComponentType = itk::ImageIOBase::FLOAT;
  
  itktools::ComponentType outputComponentType = itktools::GetImageComponentType(componentTypeOutString);

    
  try
  {    
    // now call all possible template combinations.
    if (!texture) texture = ITKToolsTexture< float, float, 2 >::New( inputComponentType, outputComponentType, dim );
    if (!texture) texture = ITKToolsTexture< float, double, 2 >::New( inputComponentType, outputComponentType, dim );
    
#ifdef ITKTOOLS_3D_SUPPORT
    if (!texture) texture = ITKToolsTexture< float, float, 3 >::New( inputComponentType, outputComponentType, dim );    
    if (!texture) texture = ITKToolsTexture< float, double, 3 >::New( inputComponentType, outputComponentType, dim );
#endif
    if (!texture) 
    {
      std::cerr << "ERROR: this combination of pixeltype and dimension is not supported!" << std::endl;
      std::cerr
        << "input pixel (component) type = " << inputComponentType
        << "output pixel (component) type = " << outputComponentType
        << " ; dimension = " << Dimension
        << std::endl;
      return 1;
    }

    texture->m_InputFileName = inputFileName;
    texture->m_OutputDirectory = outputDirectory;
    texture->m_NeighborhoodRadius = neighborhoodRadius;
    texture->m_OffsetScales = offsetScales;
    texture->m_NumberOfBins = numberOfBins;
    texture->m_NumberOfOutputs = numberOfOutputs;
  
    texture->Run();
    
    delete texture;  
  }
  catch( itk::ExceptionObject &e )
  {
    std::cerr << "Caught ITK exception: " << e << std::endl;
    delete texture;
    return 1;
  }

  /** End program. */
  return 0;

} // end main()
